#ifndef SSHUTTLED_LOG_H
#define SSHUTTLED_LOG_H

#include <sys/syslog.h>
#include <stdbool.h>
#include <stdarg.h>

extern const char* log_identifier;
extern const char* log_filename;

extern bool is_syslog_open;
extern bool is_logfile_open;

void log_open_syslog(const char* identifier);
void log_open_logfile(const char* filename);

void log_close_syslog();
void log_close_logfile();

void log_message(int priority, const char* fmt, ...);
void log_flush();

void log_message_logfile_v(int priority, const char* fmt, va_list args);
void log_message_syslog_v(int priority, const char* fmt, va_list args);

void log_message_logfile(int priority, const char* fmt, ...);
void log_message_syslog(int priority, const char* fmt, ...);

#endif
