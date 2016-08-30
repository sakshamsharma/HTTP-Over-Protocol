#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "logger.cpp"

#ifndef PROXY
#define PROXY
#include "proxysocket.h"
#endif

using namespace std;

ServerSocket mainSocket;
char *remoteUrl;
int remotePort;

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

    ProxySocket outsock = ProxySocket();
    outsock.MakeOutwardConnection(remoteUrl, remotePort, PLAIN);

    bool areTheyStillThere = true;

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
        }

    } while (areTheyStillThere);
}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 4)
        error("Usage format: ./http-server <local port> <remote url> <remotePort>");
    portNumber = atoi(argv[1]);
    remoteUrl = argv[2];
    remotePort = atoi(argv[3]);

    // Class ServerSocket handles the connection logic
    mainSocket.listenOnPort(portNumber);

    // The main loop which receives and handles connections
    while (1) {
        // Accept connections and create a class instance
        // Forks as needed
        mainSocket.connectToSocket(exchangeData);
    }
    return 0;
}
