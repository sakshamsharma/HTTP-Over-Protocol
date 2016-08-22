#include "standard.h"
#include "utils.h"
#include "serversocket.h"
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

void handleConnection(ClientSocket csock) {
    cout << "Haha" << endl;
    vector<char> buffer((BUFSIZE+5)*sizeof(char));

    csock.readIntoBuffer(buffer);
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
