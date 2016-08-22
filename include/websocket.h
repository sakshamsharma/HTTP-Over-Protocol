#include "standard.h"
#include "proxy_parse.h"

class WebSocket {
public:
    int fd, n;
    struct sockaddr_in servAddr;
    struct hostent *server;

    char webbuffer[(BUFSIZE+5) * sizeof(uchar)];

    WebSocket(char *host, int portNumber);
    int recvOnSocket(std::vector<char> &buffer);
    int sendRequest(struct ParsedRequest *pr);
    void closeSocket();
};
