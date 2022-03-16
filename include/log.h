#ifndef SSHUTTLED_LOG_H
#define SSHUTTLED_LOG_H

void log_open(const char* identifier, const char* file_name);
void log_close();

void log_flush();
void log_message(int priority, const char* fmt, ...);

#endif
