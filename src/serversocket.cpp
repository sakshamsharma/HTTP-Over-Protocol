#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "proxysocket.h"

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

void ServerSocket::connectToSocket(void (*connectionCallback)(ProxySocket&),
                                   Modes mode) {

    int c = accept(mainSocketFd, (struct sockaddr *) &client, &clientLen);
    if (c < 0) return;
    //setNonBlocking(c);

    ProxySocket sock = ProxySocket(c, mode==CLIENT?PLAIN:HTTP);

    info("Connected to client");

    pid = fork();
    if (pid < 0) {
        error("Could not fork");
    } else if (pid == 0) {
        // Forked process
        this->closeSocket();

        connectionCallback(sock);
        info("Closing connection");
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
