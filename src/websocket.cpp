#include "standard.h"
#include "utils.h"
#include "proxy_parse.h"
#include "websocket.h"

WebSocket::WebSocket(char *host, int portNumber) {
    // Open a socket file descriptor
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) error("Could not open socket");

    // Standard syntax
    server = gethostbyname(host);
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

    setNonBlocking(fd);
}

int WebSocket::sendRequest(struct ParsedRequest *pr) {
    ParsedHeader_remove(pr, "Connection");
    ParsedHeader_remove(pr, "connection");
    ParsedHeader_remove(pr, "Host");
    int status = 0;

    n = 0;
    n += sprintf(webbuffer+n, "GET %s HTTP/1.0\r\n", pr->path);

    status = ParsedHeader_printHeaders(pr, webbuffer+n, BUFSIZE-n-1);
    if (status != 0) {
        warn("Bad request header size");
        return 1;
    }
    n += ParsedHeader_headersLen(pr);
    if (webbuffer[n-1] == '\n' &&
        webbuffer[n-2] == '\r' &&
        webbuffer[n-3] == '\n' &&
        webbuffer[n-4] == '\r') {
        n -= 2;
    }
    n += sprintf(webbuffer+n, "Host: %s\r\n", pr->host);
    n += sprintf(webbuffer+n, "Connection: close\r\n\r\n");

    send(fd, webbuffer, n, 0);

#ifdef DEBUG
    std::cout << webbuffer << std::endl;
#endif

    return 0;
}

int WebSocket::recvOnSocket(std::vector<char> &buffer, int n) {
    return recv(fd, &buffer[n], BUFSIZE - n - 2, 0);
}

void WebSocket::closeSocket() {
    if (writeSocket != NULL) {
        fclose(writeSocket);
    }
    if (readSocket != NULL) {
        fclose(readSocket);
    }
    close(fd);
}

int WebSocket::sendOnSocket(std::vector<char> &buffer, int n) {
    return send(fd, &buffer[0], n, 0);
}

int WebSocket::writeOnSocket(std::vector<char> &buffer, int n) {
    if (writeSocket == NULL) {
        writeSocket = fdopen(dup(fd), "w");
    }
    return fwrite(&buffer[0], 1, n, writeSocket);
}
