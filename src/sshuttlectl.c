#include "log.h"

#include <stdlib.h>

int main() {
  log_open_logfile(NULL);
  log_message(LOG_INFO, "Correct setup");

  log_close_logfile();
  return EXIT_SUCCESS;
}
