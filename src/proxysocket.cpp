#include "standard.h"
#include "utils.h"
#include "proxysocket.h"
#include "logger.h"

using namespace std;

ProxySocket::ProxySocket(int _fd, Protocol _inProto) {
    fd = _fd;
    protocol = _inProto;
    logger(DEBUG) << "Accepted connection";
    logger(DEBUG) << "This connection is in " << (protocol==PLAIN?"PLAIN":"HTTP");
    sprintf(ss, "HTTP/1.1 200 OK\r\nContent-Length");

    setNonBlocking(fd);
}

ProxySocket::ProxySocket(char *host, int port, Protocol _outProto) {
    protocol = _outProto;
    logger(DEBUG) << "Making outgoing connection to " << host << ":" << port;
    logger(DEBUG) << "This connection is in " << (protocol==PLAIN?"PLAIN":"HTTP");
    sprintf(ss, "GET %s / HTTP/1.0\r\nHost: %s:%d\r\nContent-Length",
            host, host, port);

    // Open a socket file descriptor
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        logger(ERROR) << "Could not open outward socket";
        exit(0);
    }

    // Standard syntax
    server = gethostbyname(host);
    if (!server) {
        logger(ERROR) << "Cannot reach host";
        exit(0);
    }

    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&servAddr.sin_addr.s_addr,
          server->h_length);
    servAddr.sin_port = htons(port);

    // Connect to the server's socket
    if (connect(fd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        logger(ERROR) << "Was connecting to " << host << ":" << port << "\n"
                      << "Cannot connect to remote server";
        exit(0);
    }

    setNonBlocking(fd);
}

int ProxySocket::recvFromSocket(vector<char> &buffer, int from,
                                int &respFrom) {

    a = 0;
    b = 0;
    connectionBroken = false;
    gotHttpHeaders = -1;
    if (protocol == PLAIN) {
        logger(DEBUG) << "Recv PLAIN";
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2, 0);
            if (retval < 0) {
                b++;
            } else {
                if (retval == 0) {
                    connectionBroken = true;
                    break;
                }
                b = 0;
                a += retval;
            }
        } while (b < 50000 && a < 500);
        respFrom = 0;

        logger(DEBUG) << "Received " << a << " bytes as plain.";

    } else if (protocol == HTTP) {
        logger(DEBUG) << "Recv HTTP";
        k = 0;
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2-a, 0);
            if (retval == -1) {
                k++;
            } else if (retval == 0) {
                connectionBroken = true;
                break;
            } else {
                k = 0;
                a += retval;
                for (b=from; b<a+from-3; b++) {
                    if (buffer[b] == '\r' && buffer[b+1] == '\n' &&
                        buffer[b+2] == '\r' && buffer[b+3] == '\n') {
                        b += 4;
                        gotHttpHeaders = b-from;
                        break;
                    }
                }
            }
        } while (gotHttpHeaders == -1 && k < 50000);

        if (connectionBroken == true) {
            return -1;
        } else if (a == 0) {
            return 0;
        }

        // If it was a normal HTTP response,
        // we should parse it
        logger(DEBUG) << "Received " << a << " bytes as HTTP headers";
        logger(DEBUG) << &buffer[from];

        // Find content length header
        // TODO Make this more optimum
        for (b=from; b<a+from; b++) {
            if (strncmp(&buffer[b], "Content-Length: ", 16) == 0) {
                break;
            }
        }

        // If we couldn't find the header
        if (b == a+from) {
            logger(DEBUG) << "Didn't find content-length in headers";
            return 0;
        }

        // Point @b to the start of the content length int
        b += 17;
        int tp=0;
        while (buffer[b] >= '0' && buffer[b] <= '9') {
            tp *= 10;
            tp += buffer[b]-'0';
            b++;
        }

        from = a+from;
        a = a-gotHttpHeaders;
        respFrom = gotHttpHeaders;
        logger(DEBUG) << "headers end at " << gotHttpHeaders;

        // Read the response
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2-a, 0);
            if (retval == 0) {
                connectionBroken = true;
                break;
            }
            if (retval > 0) {
                a += retval;
            }
        } while (a < tp);
    }

    // Signal error, or return length of message
    return connectionBroken ? -1 : a;
}

int ProxySocket::sendFromSocket(vector<char> &buffer, int from, int len) {

    a = 0;
    b = 0;
    if (protocol == PLAIN) {
        logger(DEBUG) << "Write PLAIN";
        a = send(fd, &buffer[from], len, 0);
    } else if (protocol == HTTP) {
        logger(DEBUG) << "Write HTTP";
        b = sprintf(headers, "%s: %d\r\n\r\n", ss, len);
        logger(DEBUG) << "Wrote " << headers;
        a = send(fd, headers, b, 0);
        if (a < 1) {
            return -1;
        }
        a = send(fd, &buffer[from], len, 0);
        buffer[from+len] = 0;
        logger(DEBUG) << "Wrote now " << &buffer[from];
    }
    return a;
}

void ProxySocket::closeSocket() {
    close(fd);
}
