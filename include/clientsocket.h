class ClientSocket {
 public:
  int fd;
  char ipaddr[40];
  FILE *writeSocket, *readSocket;

  ClientSocket(int, char *);
  void closeSocket();
  void writeBufferToSocket(std::vector<char>&, int);
  int readIntoBuffer(std::vector<char>&, int);
  int recvFromSocket(std::vector<char> &buffer, int n);
  int sendOnSocket(std::vector<char> &buffer, int n);

  void send400(std::vector<char>&, char *timebuf);
  void send404(std::vector<char>&, char *timebuf, std::string&);
  void send405(std::vector<char>&, char *timebuf, std::string&);
  void send501(std::vector<char>&, char *timebuf);
};
