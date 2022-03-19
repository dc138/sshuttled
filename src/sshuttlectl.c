#include "log.h"
#include "pid.h"
#include "fifo.h"

#include <stdlib.h>
#include <stdio.h>

static fifo_t server_fifo;

int main() {
  log_open_logfile(NULL);

  if (!pid_is_program_open("/var/run/sshuttled/pid")) {
    log_message(LOG_ERR, "sshuttled is not running");
    return EXIT_FAILURE;
  }

  fifo_create_existing(&server_fifo, "/var/run/sshuttled/in");

  log_close_logfile();
  return EXIT_SUCCESS;
}
