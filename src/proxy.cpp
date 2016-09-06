#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "proxysocket.h"
#include "logger.h"

#define SLEEPT 100000

using namespace std;

ServerSocket mainSocket;
char *remoteUrl;
int remotePort;
Mode mode = CLIENT;

volatile bool remoteToListenerOn = false;
volatile bool listenerToRemoteOn = false;

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

struct connectionSockets {
    ProxySocket& csock;
    ProxySocket& outsock;
};

void *remoteToListener(void *context) {
    vector<char> outBuffer((BUFSIZE+5)*sizeof(char));
    int failuresOut = 0;
    int a, b, c;

    struct connectionSockets *ptr = (struct connectionSockets*)context;
    ProxySocket& csock = ptr->csock;
    ProxySocket& outsock = ptr->outsock;

    do {
        // @a stores the number of bytes in the message
        // @b is passed by reference
        // It will store the position where the actual
        // message starts.
        // This is 0 if the message was in plaintext
        // but will vary in case of HTTP
        a = outsock.recvFromSocket(outBuffer, 0, b);
        logger(VERB1) << "Got " << a << " bytes on outsocket";
        if (a == -1) {
            // Connection has been broken
            failuresOut++;
        } else if (a == 0) {
            logger(DEBUG) << "Got nothing from remote";
            failuresOut = 0;
        } else {
            // TODO If sock is HTTP, don't send till you get a request
            c = csock.sendFromSocket(outBuffer, b, a);
            logger(VERB1) << "Wrote " << c << " bytes on insocket";
            if (c == -1) {
                failuresOut++;
            } else {
                failuresOut = 0;
            }
            logger(DEBUG) << "Sent " << c << "/" << a << " bytes from remote to local";
        }
        outBuffer[0] = 0;       // To allow sane logging
        if (failuresOut != 0) {
            logger(WARN, "RTL") << "Input failures: " << failuresOut;
        }
        usleep(SLEEPT);
    } while (failuresOut < 10 && listenerToRemoteOn == true);
    logger(WARN, "RTL") << "Exiting";
    remoteToListenerOn = false;
}

void *listenerToRemote(void *context) {
    vector<char> inpBuffer((BUFSIZE+5)*sizeof(char));
    int failuresIn = 0;
    int a, b, c;

    struct connectionSockets *ptr = (struct connectionSockets*)context;
    ProxySocket& csock = ptr->csock;
    ProxySocket& outsock = ptr->outsock;

    do {
        // @a stores number of bytes in message
        // @b is passed by reference
        a = csock.recvFromSocket(inpBuffer, 0, b);
        logger(VERB1) << "Got " << a << " bytes from insocket";
        if (a == -1) {
            // Connection has been broken
            failuresIn++;
        } else if (a == 0) {
            logger(DEBUG) << "Got nothing from client";
            failuresIn = 0;
            // TODO Send empty HTTP requests if outsock is HTTP
        } else {
            c = outsock.sendFromSocket(inpBuffer, b, a);
            logger(VERB1) << "Wrote " << c << " bytes into outsocket";
            logger(DEBUG) << "Sent " << c << "/" << a << " bytes from local to remote";
            if (c == -1) {
                failuresIn++;
            } else {
                failuresIn = 0;
            }
        }
        inpBuffer[0] = 0;       // For sane logging
        if (failuresIn != 0) {
            logger(WARN, "LTR") << "Input failures: " << failuresIn;
        }
        usleep(SLEEPT);
    } while (failuresIn < 10 && remoteToListenerOn == true);
    logger(WARN, "LTR") << "Exiting";
    listenerToRemoteOn = false;
}

void exchangeData(ProxySocket& sock) {

    // This socket is HTTP for clients
    // but PLAIN for the server process
    // Server process talks to the SSH server
    // But Client process talks to the evil proxy
    ProxySocket outsock = ProxySocket(remoteUrl, remotePort,
                                      mode==CLIENT?HTTP:PLAIN);

    if (mode == CLIENT) {
        logger(VERB1) << "Receiving hello handshake";
        outsock.sendHelloMessage();
        logger(VERB1) << "Received handshake";
    } else {
        logger(VERB1) << "Sending hello handshake";
        sock.receiveHelloMessage();
        logger(VERB1) << "Sent handshake";
    }

    remoteToListenerOn = true;
    listenerToRemoteOn = true;

    static struct connectionSockets context = {
        sock, outsock
    };

    logger(VERB1) << "Ready to spawn read-write workers";

    pthread_t thread1, thread2;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&thread1, &attr, remoteToListener, &context);
    pthread_create(&thread1, &attr, listenerToRemote, &context);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 4 && argc != 5) {
        logger(ERROR) <<
            "Usage format: ./hop <local port> <remote url> <remotePort>";
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
