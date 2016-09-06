#ifndef PROXY
#define PROXY

#include "standard.h"

enum Protocol { PLAIN, HTTP };
enum Mode { CLIENT, SERVER };

class ProxySocket {
public:
    int fd;
    struct sockaddr_in servAddr;
    struct hostent *server;
    char ss[MAXHOSTBUFFERSIZE+1];
    char headers[MAXHOSTBUFFERSIZE+5];
    volatile int lock;

    Protocol protocol;

    int retval, gotHttpHeaders, k;
    int receivedBytes, sentBytes, writtenBytes, readBytes, numberOfFailures;
    bool connectionBroken;
    int contentBytes, startOfContent, bufferBytesRemaining, contentLength;

    ProxySocket(int, Protocol);
    ProxySocket(char *, int, Protocol);

    int recvFromSocket(std::vector<char> &buffer, int, int &);
    int sendFromSocket(std::vector<char> &buffer, int len, int from);

    void sendHelloMessage();
    void receiveHelloMessage();

    void closeSocket();
};

#endif
