#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static int   running       = 0;
static int   delay         = 1;
static int   counter       = 0;
static char *pid_file_name = "pid.pid";
static int   pid_fd        = -1;
static char *app_name      = NULL;

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    log_message(LOG_DEBUG, "Received termination signal\n");

    if (pid_fd != -1) {
      lockf(pid_fd, F_ULOCK, 0);
      close(pid_fd);
    }

    if (pid_file_name != NULL) {
      unlink(pid_file_name);
    }

    running = 0;
    signal(sig, SIG_DFL);

  } else if (sig == SIGCHLD) {
    log_message(LOG_DEBUG, "Received SIGCHLD signal\n");
  }
}

static void daemonize() {
  pid_t pid = 0;
  int   fd;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }

  signal(SIGCHLD, SIG_IGN);

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  // chdir("/");

  for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    close(fd);
  }

  stdin  = fopen("/dev/null", "r");
  stdout = fopen("/dev/null", "w+");
  stderr = fopen("/dev/null", "w+");

  if (pid_file_name != NULL) {
    char str[256];
    pid_fd = open(pid_file_name, O_RDWR | O_CREAT, 0640);

    if (pid_fd < 0) {
      exit(EXIT_FAILURE);
    }

    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
      exit(EXIT_FAILURE);
    }

    sprintf(str, "%d\n", getpid());
    write(pid_fd, str, strlen(str));
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

  int   value, option_index = 0;
  char *log_file_name    = "log.log";
  bool  start_daemonized = true;

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
  }

  log_open(app_name, start_daemonized == true ? log_file_name : NULL);

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
