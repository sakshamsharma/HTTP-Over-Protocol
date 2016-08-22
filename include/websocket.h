#include "standard.h"

class WebSocket {
public:
    int fd, n;
    struct sockaddr_in servAddr;
    struct hostent *server;

    char webbuffer[(BUFSIZE+5) * sizeof(uchar)];

    WebSocket(std::string &host, int portNumber);
    void sendRequest(struct ParsedHeader *ph);
    void closeSocket();
};
