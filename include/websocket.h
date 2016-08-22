class WebSocket {
 public:
  int fd;
  struct sockaddr_in servAddr;
  struct hostent *server;

  WebSocket(std::string &host, int portNumber);
  void sendRequest(struct ParsedHeader *ph);
  void closeSocket();
};
