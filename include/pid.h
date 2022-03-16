#ifndef SSHUTTLED_PID_H
#define SSHUTTLED_PID_H

#include <stdbool.h>

extern const char* pid_filename;
extern bool        is_pid_created;

void pid_create(const char* filename);
void pid_delete();

#endif
