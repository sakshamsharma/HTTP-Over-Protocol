#include "standard.h"
#include "proxy_parse.h"

class WebSocket {
public:
    int fd, n;
    FILE *writeSocket, *readSocket;
    struct sockaddr_in servAddr;
    struct hostent *server;

    char webbuffer[(BUFSIZE+5) * sizeof(uchar)];

    WebSocket(char *host, int portNumber);
    int recvOnSocket(std::vector<char> &buffer, int n);
    int sendRequest(struct ParsedRequest *pr);
    int sendOnSocket(std::vector<char> &buffer, int n);
    void closeSocket();
    int writeOnSocket(std::vector<char> &buffer, int n);
};
