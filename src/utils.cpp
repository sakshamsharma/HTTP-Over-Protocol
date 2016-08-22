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
