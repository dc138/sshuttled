#ifndef SSHUTTLED_LOG_H
#define SSHUTTLED_LOG_H

#include <sys/syslog.h>
#include <stdbool.h>

extern const char* log_identifier;
extern const char* log_filename;
extern bool        is_log_open;

void log_open(const char* identifier, const char* file_name);
void log_close();

void log_flush();
void log_message(int priority, const char* fmt, ...);

#endif
