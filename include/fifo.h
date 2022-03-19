#ifndef SSHUTTLED_FIFO_H
#define SSHUTTLED_FIFO_H

#include <stdbool.h>

typedef struct {
  const char* filepath;
  bool        open;
} fifo_t;

void fifo_create(fifo_t* fifo, const char* filepath);
void fifo_delete(fifo_t* fifo);

bool fifo_read(fifo_t* fifo, void* buffer, int bytes);
void fifo_write(fifo_t* fifo, const char* text);

#endif
