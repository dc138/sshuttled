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

void fifo_create(fifo_t* fifo, const char* filepath) {
  assert(!fifo->open && "Fifo must not be open");

  if (mkfifo(filepath, 0666) == -1 && errno != EEXIST) {
    log_message(LOG_ERR, "Error while trying to create fifo %s, %s", filepath, strerror(errno));
  }

  log_message(LOG_INFO, "Created fifo %s", filepath);

  fifo->filepath = filepath;
  fifo->open     = true;
}

void fifo_create_existing(fifo_t* fifo, const char* filepath) {
  assert(!fifo->open && "Fifo must not be open");

  struct stat st;
  stat(filepath, &st);

  if (S_ISFIFO(st.st_mode) == 0) {
    log_message(LOG_ERR, "File %s is not a fifo", filepath);
  }

  log_message(LOG_INFO, "Using existing fifo %s", filepath);

  fifo->filepath = filepath;
  fifo->open     = true;
}

void fifo_delete(fifo_t* fifo) {
  assert(fifo->open && "Fifo must be open");

  if (remove(fifo->filepath) == -1) {
    log_message(LOG_WARNING, "Error while trying to delete fifo %s, %s", fifo->filepath, strerror(errno));
  }

  log_message(LOG_INFO, "Deleted fifo %s", fifo->filepath);

  fifo->open     = false;
  fifo->filepath = NULL;
}

bool fifo_read(fifo_t* fifo, void* buffer, int bytes) {
  assert(fifo->open && "Fifo must be open");
  int fd = open(fifo->filepath, O_RDONLY);

  if (fd < 0) {
    log_message(LOG_ERR, "Cannot open fifo %s for reading", fifo->filepath);
  }

  int res = read(fd, buffer, bytes);
  close(fd);

  return res == 0 ? false : true;
}

void fifo_write(fifo_t* fifo, const char* text) {
  assert(fifo->open && "Fifo must be open");
  int fd = open(fifo->filepath, O_WRONLY);

  if (fd < 0) {
    log_message(LOG_ERR, "Cannot open fifo %s for writing", fifo->filepath);
  }

  write(fd, text, sizeof(text));
  close(fd);
}
