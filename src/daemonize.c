#include "daemon.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char *pid_file_name;
extern int   pid_fd;

void daemonize() {
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
