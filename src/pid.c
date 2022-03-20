#include "pid.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

const char* pid_filename   = "";
bool        is_pid_created = false;

static int pid_fd = -1;

void pid_create(const char* filename) {
  pid_filename = filename;

  if (pid_filename != NULL) {
    pid_fd = open(pid_filename, O_RDWR | O_CREAT, 0640);

    if (pid_fd < 0) {
      log_message(LOG_ERR, "Cannot open() pid file %s, error: %s", pid_filename, strerror(errno));
    }

    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
      log_message(LOG_ERR, "Cannot lock() pid file %s, error: %s", pid_filename, strerror(errno));
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

bool pid_is_program_open(const char* filename) {
  FILE* pid_file = fopen(filename, "r");

  if (pid_file == NULL) {
    return false;
  }

  char pid_string[32];
  fgets(pid_string, 32, pid_file);
  fclose(pid_file);

  if (pid_string[strlen(pid_string) - 1] == '\n') {
    pid_string[strlen(pid_string) - 1] = 0;
  }

  const char* proc_fmt = "/proc/%s/";
  char        proc_folder[512];

  sprintf(proc_folder, proc_fmt, pid_string);

  DIR* proc_dir = opendir(proc_folder);

  if (errno == ENOENT) {
    return false;
  }

  closedir(proc_dir);
  return true;
}
