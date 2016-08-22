#include "standard.h"
#include "utils.h"
#include "serversocket.h"

#ifndef CLIENTS
#define CLIENTS
#include "clientsocket.h"
#endif

ServerSocket::ServerSocket() {
    on = 1;
    clientLen = sizeof(client);
}

// Setup the basic server and listening logic
void ServerSocket::listenOnPort(int portNumber) {
    mainSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mainSocketFd < 0) error("Could not open socket");


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portNumber);

    /* Binding the newly created socket to the server address and port */
    if (bind(mainSocketFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        error("Could not bind to socket");
    listen(mainSocketFd, 30000);
}

// Accept connections and return a ClientSocket object
void ServerSocket::connectToClient(void (*connectionCallback)(ClientSocket)) {

    int c = accept(mainSocketFd, (struct sockaddr *) &client, &clientLen);
    if (c < 0) error("Could not open connection");

    char ipaddr[40];
    inet_ntop(AF_INET, &(client.sin_addr), ipaddr, 40);
    ClientSocket csock = ClientSocket(c, ipaddr);

    info("Connected to client");

    pid = fork();
    if (pid < 0) {
        error("Could not fork");
    } else if (pid == 0) {
        // Forked process
        this->closeSocket();

        connectionCallback(csock);
        info("Closing connection");
        csock.closeSocket();
        exit(0);

    } else {
        // Main process
        csock.closeSocket();
    }

}

void ServerSocket::closeSocket() {
    close(mainSocketFd);
}
