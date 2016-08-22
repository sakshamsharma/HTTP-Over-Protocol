#include "standard.h"
#include "utils.h"
#include "serversocket.h"
#include "websocket.h"
#include "proxy_parse.h"

using namespace std;

ServerSocket mainSocket;

// For closing the sockets safely when Ctrl+C SIGINT is received
void intHandler(int dummy) {
  info("\nClosing socket\n");
  mainSocket.closeSocket();
  exit(0);
}

// To ignore SIGPIPE being carried over to the parent
// When client closes pipe
void pipeHandler(int dummy) {
  info("Connection closed due to SIGPIPE");
}

void handleConnection(ClientSocket& csock) {
    int n = 0;
    vector<char> buffer((BUFSIZE+5)*sizeof(char));
    char timebuf[80];
    fillTimeBuffer(timebuf);

    do {
        n += csock.readIntoBuffer(buffer, n);
        if (n > BUFSIZE) {
            csock.closeSocket();
            error("Request length exceeded BUFSIZE");
        }
    } while (!(buffer[n-1] == '\n' &&
               buffer[n-2] == '\r' &&
               buffer[n-3] == '\n' &&
               buffer[n-4] == '\r'));

    struct ParsedRequest *parsedReq = ParsedRequest_create();
    n = ParsedRequest_parse(parsedReq, &buffer[0], BUFSIZE);

    if (n < 0) {
        csock.send400(buffer, timebuf);
        return;
    }

#ifdef VERBOSE
    cout << parsedReq->path << endl;
    cout << "Host: " << parsedReq->host << endl;
#endif
#ifdef DEBUG
    cout << parsedReq->path << endl;
    cout << "Host: " << parsedReq->host << endl;
#endif

    WebSocket wreq = WebSocket(parsedReq->host, 80);
    n = wreq.sendRequest(parsedReq);
    if (n < 0) {
        csock.send400(buffer, timebuf);
        wreq.closeSocket();
        return;
    }

    n = wreq.recvOnSocket(buffer);
    if (n < 0) {
        wreq.closeSocket();
        return;
    }

#ifdef DEBUG
    cout << "********\n";
    cout << &buffer[0] << endl;
    cout << "********\n";
#endif

    csock.writeBufferToSocket(buffer, n);

    wreq.closeSocket();

}

int main(int argc, char * argv[]) {
    int portNumber, pid;

    signal(SIGINT, intHandler);
    signal(SIGPIPE, pipeHandler);
    signal(SIGCHLD, SIG_IGN);

    // CLI argument parsing
    if (argc != 2)
        error("Port number not provided\nUsage format: ./http-server <port>");
    portNumber = atoi(argv[1]);

    // Class ServerSocket handles the connection logic
    mainSocket.listenOnPort(portNumber);

    // The main loop which receives and handles connections
    while (1) {
        // Accept connections and create a class instance
        // Forks as needed
        mainSocket.connectToClient(handleConnection);
    }
    return 0;
}
