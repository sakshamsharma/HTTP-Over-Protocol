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

    ProxySocket outsock = ProxySocket(remoteUrl, remotePort,
                                      mode==CLIENT?HTTP:PLAIN);

    bool areTheyStillThere = true;
    setNonBlocking(sock.fd);

    int a;

    do {
        a = outsock.recvFromSocket(buffer, 0);
        if (a == -1) {
            areTheyStillThere = false;
            break;
        }
        if (a == 0) {
            logger << "Got nothing from remote";
        } else {
            sock.sendFromSocket(buffer, 0, a);
            logger << "Sent " << a << " bytes from remote to local";
        }

        a = sock.recvFromSocket(buffer, 0);
        if (a == -1) {
            areTheyStillThere = false;
            break;
        }
        if (a == 0) {
            logger << "Got nothing from client";
        } else {
            outsock.sendFromSocket(buffer, 0, a);
            logger << "Sent " << a << " bytes from local to remote";
        }

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
