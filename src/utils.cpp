#include "utils.h"

void error(char const *errorMsg) {
  fprintf(stderr, "[ERROR] %s\n", errorMsg);
  exit(1);
}

void info(char const *message) {
  fprintf(stdout, "[INFO ] %s\n", message);
}

void warn(char const *message) {
  fprintf(stdout, "[WARN ] %s\n", message);
}

// Given a char buffer, fills it with the standard
// HTTP time formatted string for the current time
void fillTimeBuffer(char *timebuf) {
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = gmtime (&rawtime);

  strftime (timebuf,80,"%a, %d %b %Y %T %Z",timeinfo);
}
