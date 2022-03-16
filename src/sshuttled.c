#include "log.h"
#include "daemon.h"
#include "pid.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>

static int running = 0;
static int delay   = 1;
static int counter = 0;

static char *app_name = NULL;

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    log_message(LOG_INFO, "Received termination signal\n");

    running = 0;
    signal(sig, SIG_DFL);

    pid_delete();

  } else if (sig == SIGCHLD) {
    log_message(LOG_DEBUG, "Received SIGCHLD signal\n");
  }
}

void print_help(void) {
  printf("\n Usage: %s [OPTIONS]\n\n", app_name);
  printf("  Options:\n");
  printf("   -h --help                 Print this help\n");
  printf("   -n --no-daemon            Don't daemonize\n");
  printf("\n");
}

int main(int argc, char *argv[]) {
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

  if (start_daemonized) {
    printf("Forking to background...\n");
    daemonize();
    pid_create("pid.pid");
  }

  log_open(app_name,
           start_daemonized == true ? "log.log" : NULL);  // When not running as daemon, write logs to stdout

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  running = 1;

  while (running == 1) {
    log_message(LOG_DEBUG, "%d\n", counter++);
    log_flush();

    sleep(delay);
  }

  log_close();

  return EXIT_SUCCESS;
}
