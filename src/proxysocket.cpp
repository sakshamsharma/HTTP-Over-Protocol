#include "standard.h"
#include "utils.h"
#include "proxysocket.h"
#include "logger.cpp"

using namespace std;

ProxySocket::ProxySocket(int _fd, Protocol _inProto) {
    fd = _fd;
    protocol = _inProto;
    logger << "Accepted connection";
    logger << "This connection is in " << (protocol==PLAIN?"PLAIN":"HTTP");
    sprintf(ss, "HTTP/1.1 200 OK\r\nContent-Length");

    setNonBlocking(fd);
}

ProxySocket::ProxySocket(char *host, int port, Protocol _outProto) {
    protocol = _outProto;
    logger << "Making outgoing connection to " << host << ":" << port;
    logger << "This connection is in " << (protocol==PLAIN?"PLAIN":"HTTP");
    sprintf(ss, "GET %s / HTTP/1.0\r\nHost: %s:%d\r\nContent-Length",
            host, host, port);

    // Open a socket file descriptor
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) error("Could not open outward socket");

    // Standard syntax
    server = gethostbyname(host);
    if (!server) error("Cannot reach host");

    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&servAddr.sin_addr.s_addr,
          server->h_length);
    servAddr.sin_port = htons(port);

    // Connect to the server's socket
    if (connect(fd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        logger << "Was connecting to " << host << ":" << port;
        error("Cannot connect to remote server");
    }

    setNonBlocking(fd);
}

int ProxySocket::recvFromSocket(vector<char> &buffer, int from) {

    a = 0;
    b = 0;
    connectionBroken = false;
    gotHttpHeaders = -1;
    if (protocol == PLAIN) {
        logger << "Recv PLAIN";
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2, 0);
            if (retval < 0) {
                b++;
            } else {
                //logger << "Got " << retval << " in plain.\n" << &buffer[n];
                if (retval == 0) {
                    connectionBroken = true;
                    break;
                }
                b = 0;
                a += retval;
            }
        } while (b < 50000 && a < 500);

        logger << "Received " << a << " bytes as plain.";

    } else if (protocol == HTTP) {
        logger << "Recv HTTP";
        k = 0;
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2-a, 0);
            if (retval == 0) {
                connectionBroken = true;
                break;
            }
            if (retval > 0) {
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
            if (retval == -1) {
                k++;
            }
        } while (gotHttpHeaders == -1 && k < 50000);

        if (a == 0) {
            return 0;
        }

        logger << "Received " << a << " bytes as HTTP headers";
        logger << &buffer[from];

        for (b=from; b<a+from; b++) {
            if (strncmp(&buffer[b], "Content-Length: ", 16) == 0) {
                break;
            }
        }

        if (b == a+from) {
            logger << "Didn't find content-length in headers";
            return 0;
        }

        b += 17;
        int tp=0;
        while (buffer[b] >= '0' && buffer[b] <= '9') {
            tp *= 10;
            tp += buffer[b]-'0';
            b++;
        }

        from = a+from;
        a = a-gotHttpHeaders;
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
    if (connectionBroken) {
        return -1;
    } else {
        return a;
    }
}

int ProxySocket::sendFromSocket(vector<char> &buffer, int from, int len) {

    a = 0;
    b = 0;
    if (protocol == PLAIN) {
        logger << "Write PLAIN";
        a = send(fd, &buffer[from], len, 0);
    } else if (protocol == HTTP) {
        logger << "Write HTTP";
        b = sprintf(headers, "%s: %d\r\n\r\n", ss, len);
        logger << "Wrote " << headers;
        a = send(fd, headers, b, 0);
        if (a < 1) {
            return -1;
        }
        a = send(fd, &buffer[from], len, 0);
        buffer[from+len] = 0;
        logger << "Wrote now " << &buffer[from];
    }
    return a;
}

void ProxySocket::closeSocket() {
    close(fd);
}
