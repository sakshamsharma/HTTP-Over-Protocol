#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "proxysocket.h"
#include "logger.h"

ServerSocket::ServerSocket() {
    on = 1;
    clientLen = sizeof(client);
}

// Setup the basic server and listening logic
void ServerSocket::listenOnPort(int portNumber) {
    mainSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mainSocketFd < 0) {
        logger(ERROR) << "Could not open socket";
        exit(0);
    }


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portNumber);

    /* Binding the newly created socket to the server address and port */
    if (bind(mainSocketFd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        logger(ERROR) << "Could not bind to socket";
        exit(0);
    }
    listen(mainSocketFd, 1000);
}

void ServerSocket::connectToSocket(void (*connectionCallback)(ProxySocket&),
                                   Mode mode) {

    int c = accept(mainSocketFd, (struct sockaddr *) &client, &clientLen);
    if (c < 0) return;

    ProxySocket sock = ProxySocket(c, mode==CLIENT?PLAIN:HTTP);

    logger(INFO) << "Connected to client";

    pid = fork();
    if (pid < 0) {
        logger(ERROR) << "Could not fork";
        exit(0);
    } else if (pid == 0) {
        // Forked process
        this->closeSocket();

        connectionCallback(sock);
        logger(INFO) << "Closing connection";
        sock.closeSocket();
        exit(0);

    } else {
        // Main process
        sock.closeSocket();
    }
}

void ServerSocket::closeSocket() {
    close(mainSocketFd);
}
