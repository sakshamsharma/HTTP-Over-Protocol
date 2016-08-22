#include "standard.h"
#include "utils.h"
#include "clientsocket.h"

ClientSocket::ClientSocket(int _fd, char *clientIp) {
    fd = _fd;
    strcpy(ipaddr, clientIp);
}

void ClientSocket::closeSocket() {
    close(fd);
}

// Reads text from socket into buffer
int ClientSocket::readIntoBuffer(std::vector<char> &buffer, int lenUsed) {
  int n = recv(fd, &buffer[lenUsed], BUFSIZE-lenUsed, 0);
  if (n < 0) error("Error while read message from client socket");
  return n;
}

// Writes n bytes from buffer onto socket
void ClientSocket::writeBufferToSocket(std::vector<char> &buffer, int n) {
  writeSocket = fdopen(dup(fd), "w");
  fwrite(&buffer[0], 1, n, writeSocket);
  fflush(writeSocket);
  fclose(writeSocket);
}

// Send a 400 Bad Request message
void ClientSocket::send400(std::vector<char> &buf, char *timebuf) {
  int n = 0;
  n += sprintf(&buf[n], "HTTP/1.0 400 Bad Request\r\n");
  n += sprintf(&buf[n], "Date: %s\r\n", timebuf);
  n += sprintf(&buf[n], "Server: Saksham's CS425A HTTP proxy\r\n");
  n += sprintf(&buf[n], "Connection: close\r\n");
  n += sprintf(&buf[n], "Content-type: text/html\r\n\r\n");
  n += sprintf(&buf[n], "<HEAD><TITLE>400 Bad Request</TITLE></HEAD>\r\n");
  n += sprintf(&buf[n], "<BODY><H1>400 Bad Request</H1>\r\n");
  n += sprintf(&buf[n], "The request is malformed.\r\n");
  n += sprintf(&buf[n], "</BODY>\r\n");

  writeBufferToSocket(buf, n);
}

// Send a 404 Not found message
void ClientSocket::send404(std::vector<char> &buf, char *timebuf,
                           std::string &fileName) {
  int n = 0;
  n += sprintf(&buf[n], "HTTP/1.0 404 Not Found\r\n");
  n += sprintf(&buf[n], "Date: %s\r\n", timebuf);
  n += sprintf(&buf[n], "Server: Saksham's CS425A HTTP server\r\n");
  n += sprintf(&buf[n], "Connection: close\r\n");
  n += sprintf(&buf[n], "Content-type: text/html\r\n\r\n");
  n += sprintf(&buf[n], "<HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n");
  n += sprintf(&buf[n], "<BODY><H1>404 Not Found</H1>\r\n");
  n += sprintf(&buf[n], "The requested URL %s was not found on this server.\r\n", &fileName[0]);
  n += sprintf(&buf[n], "</BODY>\r\n");

  writeBufferToSocket(buf, n);
}

// Send a 405 Method not allowed message
void ClientSocket::send405(std::vector<char> &buf, char *timebuf,
                           std::string &methodName) {
  int n = 0;
  n += sprintf(&buf[n], "HTTP/1.0 405 Method Not Allowed\r\n");
  n += sprintf(&buf[n], "Date: %s\r\n", timebuf);
  n += sprintf(&buf[n], "Allow: GET\r\n");
  n += sprintf(&buf[n], "Server: Saksham's CS425A HTTP server\r\n");
  n += sprintf(&buf[n], "Connection: close\r\n");
  n += sprintf(&buf[n], "Content-type: text/html\r\n\r\n");
  n += sprintf(&buf[n], "<HEAD><TITLE>405 Method Not Allowed</TITLE></HEAD>\r\n");
  n += sprintf(&buf[n], "<BODY><H1>405 Method Not Allowed</H1>\r\n");
  n += sprintf(&buf[n], "The requested method (%s) is not allowed. Allowed methods are: GET\r\n", &methodName[0]);
  n += sprintf(&buf[n], "</BODY>\r\n");

  writeBufferToSocket(buf, n);
}

// Send a 501 message
void ClientSocket::send501(std::vector<char> &buf, char *timebuf) {
  int n = 0;
  n += sprintf(&buf[n], "HTTP/1.0 501 Internal Server Error\r\n");
  n += sprintf(&buf[n], "Date: %s\r\n", timebuf);
  n += sprintf(&buf[n], "Server: Saksham's CS425A HTTP server\r\n");
  n += sprintf(&buf[n], "Connection: close\r\n");
  n += sprintf(&buf[n], "Content-type: text/html\r\n\r\n");
  n += sprintf(&buf[n], "<HEAD><TITLE>501 Internal Server Error</TITLE></HEAD>\r\n");
  n += sprintf(&buf[n], "<BODY><H1>501 Internal Server Error</H1>\r\n");
  n += sprintf(&buf[n], "There was an internal server error.\r\n");
  n += sprintf(&buf[n], "</BODY>\r\n");

  writeBufferToSocket(buf, n);
}
