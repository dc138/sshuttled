#include "log.h"
#include "daemon.h"
#include "pid.h"
#include "dirs.h"
#include "fifo.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>

bool running = true;
int  counter = 0;

const char* app_name    = NULL;
const char* app_version = "0.1.0";

static fifo_t fifo_in;
static fifo_t fifo_out;

void print_help(void) {
  printf("\n Usage: %s [OPTIONS]\n\n", app_name);
  printf("  Options:\n");
  printf("   -h --help                 Print this help\n");
  printf("   -n --no-daemon            Don't daemonize\n");
  printf("\n");
}

void terminate(int code) {
  log_message(LOG_INFO, "Exiting sshuttled");
  pid_delete();

  fifo_delete(&fifo_in);
  fifo_delete(&fifo_out);

  log_close_logfile();
  log_close_syslog();

  exit(code);
}

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    log_message(LOG_DEBUG, "Received termination signal");

    running = false;
    signal(sig, SIG_DFL);

    terminate(EXIT_SUCCESS);

  } else if (sig == SIGCHLD) {
    log_message(LOG_DEBUG, "Received SIGCHLD signal");
  }
}

int main(int argc, char* argv[]) {
  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'}, {"no-daemon", no_argument, 0, 'n'}, {NULL, 0, 0, 0}};

  int  value, option_index = 0;
  bool start_daemonized = true;

  app_name = argv[0];

  while ((value = getopt_long(argc, argv, "hn", long_options, &option_index)) != -1) {
    switch (value) {
      case 'n':
        start_daemonized = false;
        break;

      case 'h':
        print_help();
        return EXIT_SUCCESS;

      case '?':
        print_help();
        return EXIT_FAILURE;

      default:
        break;
    }
  }

  if (geteuid() != 0) {
    printf("Must be root to run\n");
    return EXIT_FAILURE;
  }

  if (pid_is_program_open("/var/run/sshuttled/pid")) {
    printf("sshuttled is already running\n");
    return EXIT_FAILURE;
  }

  if (start_daemonized) {
    printf("Forking to background...\n");
    daemonize();
  }

  log_open_syslog(app_name);
  log_open_logfile(start_daemonized ? "/var/log/sshuttled/log.txt" : NULL);

  dirs_create("/var/run/sshuttled", 0777);
  dirs_create("/var/log/sshuttled", 0777);

  pid_create("/var/run/sshuttled/pid");

  log_message(LOG_INFO, "Starting sshuttle daemon version %s", app_version);

  fifo_create(&fifo_in, "/var/run/sshuttled/in");
  fifo_create(&fifo_out, "/var/run/sshuttled/out");

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  running           = true;
  char command[512] = {0};

  while (running) {
    if (fifo_read_line(&fifo_in, (char*)command, 512)) {
      log_message(LOG_DEBUG, "Read %s from fifo", command);
    }

    if (strcmp(command, "stop") == 0) {
      log_message(LOG_INFO, "Recieved stop command");
      raise(SIGINT);
    }

    sleep(1);
  }

  terminate(EXIT_SUCCESS);
  return EXIT_SUCCESS;
}
