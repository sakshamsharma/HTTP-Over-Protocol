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
    char ss[100];
    char headers[100];

    Protocol protocol;

    int a, b, retval, gotHttpHeaders, k;
    bool connectionBroken;

    ProxySocket(int, Protocol);
    ProxySocket(char *, int, Protocol);

    int recvFromSocket(std::vector<char> &buffer, int, int &);
    int sendFromSocket(std::vector<char> &buffer, int len, int from);
    void closeSocket();
};

#endif
