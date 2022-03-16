#include <stdio.h>
#include <stdlib.h>
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
static FILE *log_stream;

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    fprintf(log_stream, "Debug: stopping daemon ...\n");

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
    fprintf(log_stream, "Debug: received SIGCHLD signal\n");
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

/**
 * \brief Print help for this application
 */
void print_help(void) {
  printf("\n Usage: %s [OPTIONS]\n\n", app_name);
  printf("  Options:\n");
  printf("   -h --help                 Print this help\n");
  printf("   -n --no-daemon            Don't daemonize\n");
  printf("\n");
}

/* Main function */
int main(int argc, char *argv[]) {
  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'}, {"no-daemon", no_argument, 0, 'n'}, {NULL, 0, 0, 0}};

  int   value, option_index = 0, ret;
  char *log_file_name    = "log.log";
  int   start_daemonized = 1;

  app_name = argv[0];

  while ((value = getopt_long(argc, argv, "hn", long_options, &option_index)) != -1) {
    switch (value) {
      case 'n':
        start_daemonized = 0;
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

  if (!!start_daemonized) {
    printf("Forking to background...\n");
    daemonize();
  }

  openlog(argv[0], LOG_PID | LOG_CONS, LOG_DAEMON);
  syslog(LOG_INFO, "Started %s", app_name);

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  if (log_file_name != NULL) {
    log_stream = fopen(log_file_name, "a+");

    if (log_stream == NULL) {
      syslog(LOG_ERR, "Can not open log file: %s, error: %s", log_file_name, strerror(errno));
      log_stream = stdout;
    }
  } else {
    log_stream = stdout;
  }

  running = 1;

  while (running == 1) {
    ret = fprintf(log_stream, "Debug: %d\n", counter++);

    if (ret < 0) {
      syslog(LOG_ERR,
             "Can not write to log stream: %s, error: %s",
             (log_stream == stdout) ? "stdout" : log_file_name,
             strerror(errno));
      break;
    }

    ret = fflush(log_stream);

    if (ret != 0) {
      syslog(LOG_ERR,
             "Can not fflush() log stream: %s, error: %s",
             (log_stream == stdout) ? "stdout" : log_file_name,
             strerror(errno));

      break;
    }

    /* TODO: dome something useful here */

    /* Real server should use select() or poll() for waiting at
     * asynchronous event. Note: sleep() is interrupted, when
     * signal is received. */
    sleep(delay);
  }

  if (log_stream != stdout) {
    fclose(log_stream);
  }

  syslog(LOG_INFO, "Stopped %s", app_name);
  closelog();

  return EXIT_SUCCESS;
}
