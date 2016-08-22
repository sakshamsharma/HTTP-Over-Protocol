class ClientSocket {
 public:
  int fd;
  char ipaddr[40];
  FILE *writeSocket;

  ClientSocket(int, char *);
  void closeSocket();
  void writeBufferToSocket(std::vector<char>&, int);
  int readIntoBuffer(std::vector<char>&, int);

  void send400(std::vector<char>&, char *timebuf);
  void send404(std::vector<char>&, char *timebuf, std::string&);
  void send405(std::vector<char>&, char *timebuf, std::string&);
  void send501(std::vector<char>&, char *timebuf);
};
