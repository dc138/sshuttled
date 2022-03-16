#include "daemon.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

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
}
