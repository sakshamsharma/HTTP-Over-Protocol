#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "logger.cpp"
#include "proxysocket.h"

using namespace std;

ServerSocket mainSocket;
char *remoteUrl;
int remotePort;
Modes mode = CLIENT;

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

void exchangeData(ProxySocket& sock) {
    vector<char> buffer((BUFSIZE+5)*sizeof(char));
    vector<char> duffer((BUFSIZE+5)*sizeof(char));

    ProxySocket outsock = ProxySocket(remoteUrl, remotePort,
                                      mode==CLIENT?HTTP:PLAIN);

    bool areTheyStillThere = true;
    setNonBlocking(sock.fd);

    int a, b;
    bool canIRespond = false;

    do {
        a = outsock.recvFromSocket(buffer, 0, b);
        if (a == -1) {
            areTheyStillThere = false;
            break;
        }
        if (a == 0) {
            logger << "Got nothing from remote";
        } else {
            // TODO If sock is HTTP, don't send till there's a request
            // read
            // if (sock.protocol == PLAIN || canIRespond) {
            //     sock.sendFromSocket(buffer, b, a);
            //     canIRespond = false;
            // }
            sock.sendFromSocket(buffer, b, a);
            logger << "Sent " << a << " bytes from remote to local";
        }
        buffer[0] = 0;

        a = sock.recvFromSocket(duffer, 0, b);
        if (a == -1) {
            areTheyStillThere = false;
            break;
        }
        if (a == 0) {
            logger << "Got nothing from client";
            // if (outsock.protocol == HTTP) {
            //     outsock.sendEmptyHttp();
            // }
            // TODO Send empty HTTP requests if outsock is HTTP
        } else {
            outsock.sendFromSocket(duffer, b, a);
            logger << "Sent " << a << " bytes from local to remote";
        }
        duffer[0] = 0;
        usleep(100000);
    } while (areTheyStillThere);
}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 4 && argc != 5)
        error("Usage format: ./http-server <local port> <remote url> <remotePort>");
    portNumber = atoi(argv[1]);
    remoteUrl = argv[2];
    remotePort = atoi(argv[3]);

    if (argc == 5) {
        if (strcmp(argv[4], "SERVER") == 0) {
            mode = SERVER;
            info("Running as server");
        } else {
            info("Running as client");
        }
    }

    // Class ServerSocket handles the connection logic
    mainSocket.listenOnPort(portNumber);

    // The main loop which receives and handles connections
    while (1) {
        // Accept connections and create a class instance
        // Forks as needed
        mainSocket.connectToSocket(exchangeData, mode);
    }
    return 0;
}
