#include "log.h"
#include "pid.h"
#include "fifo.h"

#include <stdlib.h>
#include <stdio.h>

static fifo_t server_fifo;

const char* app_name    = NULL;
const char* app_version = "0.1.0";

void print_help() {
  printf("\n Usage: %s [COMMAND...]\n\n", app_name);
  printf("  Available commands:\n");
  printf("    start [CONNECTION] - start a connection\n");
  printf("    stop [CONNECTION] - stop a connection\n");
  printf("\n");
}

int main(int argc, char* argv[]) {
  app_name = argv[0];

  if (argc < 2) {
    print_help();
    exit(EXIT_FAILURE);
  }

  log_open_logfile(NULL);

  if (!pid_is_program_open("/var/run/sshuttled/pid")) {
    log_message(LOG_ERR, "sshuttled is not running");
    return EXIT_FAILURE;
  }

  fifo_create_existing(&server_fifo, "/var/run/sshuttled/commands");

  log_close_logfile();
  return EXIT_SUCCESS;
}
