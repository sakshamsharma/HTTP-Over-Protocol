#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "websocket.h"
#include "proxy_parse.h"
#include "logger.cpp"

using namespace std;

ServerSocket mainSocket;
char *remoteUrl;
int remotePort;
bool isItClientSide = true;

// For closing the sockets safely when Ctrl+C SIGINT is received
void intHandler(int dummy) {
  info("\nClosing socket\n");
  mainSocket.closeSocket();
  exit(0);
}

// To ignore SIGPIPE being carried over to the parent
// When client closes pipe
void pipeHandler(int dummy) {
  info("Connection closed due to SIGPIPE");
}

void handleConnection(ClientSocket& csock) {
    int contentLen = 0;
    int a = 0, b = 0, k;
    int retval;
    vector<char> buffer((BUFSIZE+5)*sizeof(char));
    bool areTheyStillThere = true;
    bool gotFullHttp = false;

    WebSocket wsock = WebSocket(remoteUrl, remotePort);

    do {
        a = 0;
        b = 0;
        gotFullHttp = false;
        do {
            retval = wsock.recvOnSocket(buffer, a);
            if (retval < 0) {
                b++;
            } else {
                logger << &buffer[0] << "\nGot " << retval << "\n";
                if (retval == 0) {
                    areTheyStillThere = false;
                    break;
                }
                b = 0;
                a += retval;
            }
        } while (b < 50000 && a < 500);

        if (a > 0) {
            logger << "Writing " << a << " bytes back";
            if (isItClientSide) {
                // Running on client side, so have to write to
                // the SSH process
                csock.sendOnSocket(buffer, a, 0);
            } else {
                // Running on remote end
                csock.sendOnSocket(buffer, a, 0);
            }
        } else {
            logger << "Got nothing from ssh server " << a;
        }

        a = 0;
        b = 0;
        do {
            retval = csock.recvFromSocket(buffer, a);
            if (retval < 0) {
                b++;
            } else {
                logger << &buffer[0] << "\nFrom client " << retval;
                if (retval == 0) {
                    areTheyStillThere = false;
                    break;
                }
                b = 0;
                a += retval;
            }
        } while (b < 50000 && a < 500);
        if (a > 0) {
            logger << "Writing " << a << " bytes to server";
            wsock.sendOnSocket(buffer, a);
        } else {
            logger << "Nothing to write back to server " << a;
        }
        if (!areTheyStillThere) {
            break;
        }
    } while (1);
}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 4 && argc != 5)
        error("Usage format: ./http-server <local port> <remote url> <remotePort> <isItClientSide>");
    portNumber = atoi(argv[1]);
    remoteUrl = argv[2];
    remotePort = atoi(argv[3]);

    if (argc == 5) {
        isItClientSide = atoi(argv[4]);
    }

    // Class ServerSocket handles the connection logic
    mainSocket.listenOnPort(portNumber);

    // The main loop which receives and handles connections
    while (1) {
        // Accept connections and create a class instance
        // Forks as needed
        mainSocket.connectToClient(handleConnection);
    }
    return 0;
}
