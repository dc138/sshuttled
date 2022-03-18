#include "log.h"
#include "daemon.h"
#include "pid.h"
#include "dirs.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>

int running = 0;
int delay   = 1;
int counter = 0;

const char* app_name = NULL;

const char* var_run_dir = "/var/run/sshuttled/";
const char* var_log_dir = "/var/log/sshuttled/";

const char* log_path = "/var/log/sshuttled/log.log";
const char* pid_path = "/var/run/sshuttled/pid.pid";

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    log_message(LOG_INFO, "Received termination signal");

    running = 0;
    signal(sig, SIG_DFL);

    pid_delete();

  } else if (sig == SIGCHLD) {
    log_message(LOG_DEBUG, "Received SIGCHLD signal");
  }
}

void print_help(void) {
  printf("\n Usage: %s [OPTIONS]\n\n", app_name);
  printf("  Options:\n");
  printf("   -h --help                 Print this help\n");
  printf("   -n --no-daemon            Don't daemonize\n");
  printf("\n");
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

  if (pid_is_program_open(pid_path)) {
    printf("Sshuttled is already running\n");
    return EXIT_FAILURE;
  }

  log_open_syslog(app_name);

  dirs_create(var_log_dir, 0777);
  dirs_create(var_run_dir, 0777);

  if (start_daemonized) {
    printf("Forking to background...\n");

    daemonize();
    pid_create(pid_path);
  }

  log_open_logfile(start_daemonized ? log_path : NULL);

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  running = 1;

  while (running == 1) {
    log_message(LOG_DEBUG, "%d", counter++);
    log_flush();

    sleep(delay);
  }

  log_close_logfile();
  log_close_syslog();

  return EXIT_SUCCESS;
}
