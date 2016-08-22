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
    int n = 0, contentLen = 0, k;
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

    WebSocket wreq = WebSocket(parsedReq->host, atoi(parsedReq->port));
    n = wreq.sendRequest(parsedReq);
    if (n < 0) {
        csock.send400(buffer, timebuf);
        wreq.closeSocket();
        return;
    }

    n = wreq.recvOnSocket(buffer);

    // If client closes connection, 0 is sent
    if (n <= 0) {
        wreq.closeSocket();
        return;
    }

    // Flush this much content to client first
    csock.writeBufferToSocket(buffer, n);

    // Now try to find the content length in this content
    char *m = strstr(&buffer[0], "\r\nContent-Length:");
    if (m == NULL) {
        m = strstr(&buffer[0], "\r\nContent-length");
    }
    if (m == NULL) {
        m = strstr(&buffer[0], "\r\ncontent-length");
    }
    if (m == NULL) {
        contentLen = n;
    }
    k = (int)(m - &buffer[0]);

    k += 18; // Length of "\r\nContent-Length:"

    // Parse the content length
    if (buffer[k] == ' ' || buffer[k] == ':') k++;
    while (buffer[k] <= '9' && buffer[k] >= '0') {
        contentLen = contentLen*10 + (int)buffer[k] - '0';
        k++;
    }

#ifdef DEBUG
    cout << "Content-Len: " << contentLen << endl;
#endif

    // Find the end of headers
    m = strstr(&buffer[0], "\r\n\r\n");
    if (m == NULL) {
        csock.send400(buffer, timebuf);
        wreq.closeSocket();
        return;
    }

    // Start of the message part
    k = (int)(m - &buffer[0] + 4);

    n = max(0, n - k);

    while (n < contentLen) {
        k = wreq.recvOnSocket(buffer);
        n += k;
        if (n < 0) {
            wreq.closeSocket();
            return;
        }
        csock.writeBufferToSocket(buffer, k);
        if (n == 0) {           // In case client closes connection
            wreq.closeSocket();
            return;
        }
    }

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
