#include "standard.h"
#include "utils.h"
#include "websocket.h"

WebSocket::WebSocket(std::string &host, int portNumber) {
    // Open a socket file descriptor
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) error("Could not open socket");

    // Standard syntax
    server = gethostbyname(host.c_str());
    if (!server) error("Cannot reach host");

    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&servAddr.sin_addr.s_addr,
          server->h_length);
    servAddr.sin_port = htons(portNumber);

    // Connect to the server's socket
    if (connect(fd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        error("Cannot connect to remote server");
}

void WebSocket::sendRequest(struct ParsedHeader *ph) {
    n = 0;
    n += sprintf(webbuffer+n, "GET / HTTP/1.0\r\n");
    n += sprintf(webbuffer+n, "Connection: close\r\n\r\n");
    send(fd, webbuffer, n, 0);
}

void WebSocket::closeSocket() {
    close(fd);
}
