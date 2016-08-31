#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "proxysocket.h"
#include "logger.h"

using namespace std;

ServerSocket mainSocket;
char *remoteUrl;
int remotePort;
Mode mode = CLIENT;

// For closing the sockets safely when Ctrl+C SIGINT is received
void intHandler(int dummy) {
    logger(INFO) << "Closing socket";
    mainSocket.closeSocket();
    exit(0);
}

// To ignore SIGPIPE being carried over to the parent
// When client closes pipe
void pipeHandler(int dummy) {
    logger(INFO) << "Connection closed due to SIGPIPE";
}

void exchangeData(ProxySocket& sock) {
    vector<char> outBuffer((BUFSIZE+5)*sizeof(char));
    vector<char> inpBuffer((BUFSIZE+5)*sizeof(char));

    // This socket is HTTP for clients
    // but PLAIN for the server process
    // Server process talks to the SSH server
    // But Client process talks to the evil proxy
    ProxySocket outsock = ProxySocket(remoteUrl, remotePort,
                                      mode==CLIENT?HTTP:PLAIN);
    int a, b;

    do {
        // @a stores the number of bytes in the message
        // @b is passed by reference
        // It will store the position where the actual
        // message starts.
        // This is 0 if the message was in plaintext
        // but will vary in case of HTTP
        a = outsock.recvFromSocket(outBuffer, 0, b);
        if (a == -1) {
            // Connection has been broken
            return;
        } else if (a == 0) {
            logger(DEBUG) << "Got nothing from remote";
        } else {
            // TODO If sock is HTTP, don't send till you get a request
            sock.sendFromSocket(outBuffer, b, a);
            logger(DEBUG) << "Sent " << a << " bytes from remote to local";
        }
        outBuffer[0] = 0;       // To allow sane logging

        // @a stores number of bytes in message
        // @b is passed by reference
        a = sock.recvFromSocket(inpBuffer, 0, b);
        if (a == -1) {
            // Connection has been broken
            return;
        } else if (a == 0) {
            logger(DEBUG) << "Got nothing from client";
            // TODO Send empty HTTP requests if outsock is HTTP
        } else {
            outsock.sendFromSocket(inpBuffer, b, a);
            logger(DEBUG) << "Sent " << a << " bytes from local to remote";
        }
        inpBuffer[0] = 0;       // For sane logging
        usleep(100000);

    } while (1);
}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 4 && argc != 5) {
        logger(ERROR) <<
            "Usage format: ./http-server <local port> <remote url> <remotePort>";
        exit(0);
    }

    portNumber = atoi(argv[1]);
    remoteUrl = argv[2];
    remotePort = atoi(argv[3]);

    if (argc == 5) {
        if (strcmp(argv[4], "SERVER") == 0) {
            mode = SERVER;
            logger(INFO) << "Running as server";
        } else {
            logger(INFO) << "Running as client";
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
