#include "pid.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

const char* pid_filename   = "";
bool        is_pid_created = false;

static int pid_fd = -1;

void pid_create(const char* filename) {
  pid_filename = filename;

  if (pid_filename != NULL) {
    pid_fd = open(pid_filename, O_RDWR | O_CREAT, 0640);

    if (pid_fd < 0) {
      log_message(LOG_ERR, "Cannot write pid file %s", pid_filename);
    }

    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
      log_message(LOG_ERR, "Cannot lock pid file %s", pid_filename);
    }

    char str[256];
    sprintf(str, "%d\n", getpid());
    write(pid_fd, str, strlen(str));

    is_pid_created = true;
  }
}

void pid_delete() {
  if (pid_fd != -1) {
    lockf(pid_fd, F_ULOCK, 0);
    close(pid_fd);
  }

  if (pid_filename != NULL) {
    unlink(pid_filename);
  }

  is_pid_created = false;
}
