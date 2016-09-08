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

mutex tlock;

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

struct tunnelContext {
    ProxySocket& readSocket;
    ProxySocket& writeSocket;
    volatile bool& amIRunning;
    volatile bool& sleepOn;
    const char* type;
};

void *packetTunnel(void *_context) {
    struct tunnelContext *context = (struct tunnelContext*)_context;
    ProxySocket& readSocket = context->readSocket;
    ProxySocket& writeSocket = context->writeSocket;
    const char* type = context->type;
    volatile bool& otherHalfRunning = context->sleepOn;

    vector<char> buffer((BUFSIZE+5)*sizeof(char));
    int failures = 0;
    int messageSize, messageFrom;
    bool otherPartySaysFine;

    while (failures < 5 && otherHalfRunning) {
        tlock.lock();

        // messageFrom passed by reference
        messageSize = readSocket.read(buffer, 0, messageFrom);

        if (messageSize == 0) {
            // Empty message, confirm
            logger(DEBUG, type) << "Read 0 bytes";
        } else if (messageSize == -1) {
            // Connection was closed
            logger(VERB1, type) << "Reading socket was closed";
            failures++;
        } else if (messageSize == -2) {
            // Does not follow protocol
            logger(VERB1, type) << "Received bad message";
            failures += 10;
        } else {
            // Got some bytes
            logger(VERB2, type) << "Received " << messageSize << " bytes";
            logger(VERB2, type) << "Wrote " <<
                writeSocket.write(buffer, messageSize, messageFrom) << " bytes";
        }

        tlock.unlock();
        usleep(SLEEPT);
    }

    logger(WARN, type) << "Exiting";
    context->amIRunning = false;
}

void exchangeData(ProxySocket& sock) {

    // This socket is HTTP for clients
    // but PLAIN for the server process
    // Server process talks to the SSH server
    // But Client process talks to the evil proxy
    ProxySocket outsock = ProxySocket(remoteUrl, remotePort,
                                      mode==CLIENT?HTTP:PLAIN);

    // if (mode == CLIENT) {
    //     logger(VERB1) << "Sending hello handshake";
    //     outsock.sendHelloMessage();
    //     logger(VERB1) << "Sent handshake";
    // } else {
    //     logger(VERB1) << "Receiving hello handshake";
    //     sock.receiveHelloMessage();
    //     logger(VERB1) << "Received handshake";
    // }

    volatile bool ClientToOut = true;
    volatile bool OutToClient = true;

    struct tunnelContext fromClientToOut = {
        sock,
        outsock,
        OutToClient,
        ClientToOut,
        mode==CLIENT?"PlainToHTTP":"HTTPtoPlain"
    };

    struct tunnelContext fromOutToClient = {
        outsock,
        sock,
        ClientToOut,
        OutToClient,
        mode==CLIENT?"HTTPtoPlain":"PlainToHTTP"
    };

    logger(VERB1) << "Ready to spawn read-write workers";

    pthread_t thread1, thread2;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&thread1, &attr, packetTunnel, &fromClientToOut);
    pthread_create(&thread2, &attr, packetTunnel, &fromOutToClient);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    usleep(5000000);
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
