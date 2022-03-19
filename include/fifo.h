#ifndef SSHUTTLED_FIFO_H
#define SSHUTTLED_FIFO_H

#include <stdbool.h>

extern const char* fifo_filename;
extern bool        is_fifo_created;

void fifo_create(const char* filepath);
void fifo_delete();

bool fifo_read(void* buffer, int bytes);
void fifo_write(const char* text);

#endif
