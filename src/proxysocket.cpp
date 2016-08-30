#include "standard.h"
#include "utils.h"
#include "proxysocket.h"
#include "logger.cpp"

using namespace std;

ProxySocket::ProxySocket() {}

void ProxySocket::ListeningMode(int _fd, Protocol _inProto) {
    fd = _fd;
    protocol = _inProto;

    setNonBlocking(fd);
}

void ProxySocket::MakeOutwardConnection(char *host, int port, Protocol _outProto) {
    protocol = _outProto;

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
    if (connect(fd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        error("Cannot connect to remote server");

    setNonBlocking(fd);
}

int ProxySocket::recvFromSocket(vector<char> &buffer, int from) {

    a = 0;
    b = 0;
    connectionBroken = false;
    gotHttpHeaders = false;
    if (protocol == PLAIN) {
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
        do {
            retval = recv(fd, &buffer[a+from], BUFSIZE-from-2, 0);
            if (retval == 0) {
                connectionBroken = true;
                break;
            }
            if (retval > 0) {
                a += retval;
                for (b=from; b<a+from-3; b++) {
                    if (buffer[b] == '\r' && buffer[b+1] == '\n' &&
                        buffer[b+2] == '\r' && buffer[b+3] == '\n') {
                        gotHttpHeaders = true;
                        b += 4;
                        break;
                    }
                }
            }
        } while (!gotHttpHeaders);

        logger << "Received " << a << " bytes as HTTP";
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
        a = send(fd, &buffer[from], len, 0);
    }
    return a;
}

void ProxySocket::closeSocket() {
    close(fd);
}
