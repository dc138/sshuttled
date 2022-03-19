#include "fifo.h"
#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

const char* fifo_filename   = "";
bool        is_fifo_created = false;

void fifo_create(const char* filepath) {
  fifo_filename = filepath;

  if (mkfifo(filepath, 0666) == -1 && errno != EEXIST) {
    log_message(LOG_ERR, "Error while trying to create fifo %s, %s", filepath, strerror(errno));
  }

  log_message(LOG_INFO, "Created fifo %s", fifo_filename);
  is_fifo_created = true;
}

void fifo_delete() {
  assert(is_fifo_created && "Fifo file must be created");

  if (remove(fifo_filename) == -1) {
    log_message(LOG_ERR, "Error while trying to delete fifo %s, %s", fifo_filename, strerror(errno));
  }

  log_message(LOG_INFO, "Deleted fifo %s", fifo_filename);
  fifo_filename   = "";
  is_fifo_created = false;
}

bool fifo_read(void* buffer, int bytes) {
  assert(is_fifo_created && "Fifo file must be created");

  int fd = open(fifo_filename, O_RDONLY);

  if (fd < 0) {
    log_message(LOG_ERR, "Cannot open fifo %s for reading", fifo_filename);
  }

  int res = read(fd, buffer, bytes);
  close(fd);

  return res == 0 ? false : true;
}

void fifo_write(const char* text) {
  assert(is_fifo_created && "Fifo file must be created");

  int fd = open(fifo_filename, O_WRONLY);

  if (fd < 0) {
    log_message(LOG_ERR, "Cannot open fifo %s for writing", fifo_filename);
  }

  write(fd, text, sizeof(text));
  close(fd);
}
