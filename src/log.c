#include "log.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>

const char* log_identifier = "";
const char* log_filename   = "";

bool is_syslog_open  = false;
bool is_logfile_open = false;

static FILE*       log_stream = NULL;
static const char* log_priority_readable[8] =
    {"EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void log_open_syslog(const char* identifier) {
  assert(identifier != NULL && "Identifier must not be null");
  assert(is_syslog_open == false && "Syslog must not be open");

  log_identifier = identifier;
  is_syslog_open = true;

  openlog(identifier, LOG_PID | LOG_CONS, LOG_DAEMON);
  log_message(LOG_INFO, "Opened syslog");
}

void log_open_logfile(const char* filename) {
  assert(is_logfile_open == false && "Logfile must not be open");
  is_logfile_open = true;

  if (filename != NULL) {
    log_filename = filename;
    log_stream   = fopen(filename, "a+");

    if (log_stream == NULL) {
      log_filename = "(stdout)";
      log_stream   = stdout;

      log_message(
          LOG_WARNING, "Can not open log file %s, falling back to stdout, error: %s", filename, strerror(errno));

    } else {
      log_message(LOG_INFO, "Opened %s as log file", filename);
    }

  } else {
    log_filename = "(stdout)";
    log_stream   = stdout;

    log_message(LOG_INFO, "No logfile specified, using (stdout) as log stream");
  }
}

void log_close_syslog() {
  assert(is_syslog_open == true && "Syslog must be open");

  log_message(LOG_INFO, "Closing syslog");
  closelog();
  is_syslog_open = false;
}

void log_close_logfile() {
  if (is_logfile_open && log_stream != NULL && log_stream != stdout) {
    log_message(LOG_INFO, "Closing %s", log_filename);
    log_flush();
    fclose(log_stream);
  }

  is_logfile_open = false;
}

void log_flush() {
  if (is_logfile_open && log_stream != NULL && fflush(log_stream) != 0) {
    syslog(LOG_ERR, "Can not fflush() log stream: %s, error: %s", log_filename, strerror(errno));
  }
}

void log_message(int priority, const char* fmt, ...) {
  assert((is_syslog_open == true || is_logfile_open == true) && "At least one log channel must be open");
  assert(priority >= 0 && priority < 8 && "Invalid priority value");

  va_list args;

  va_start(args, fmt);
  log_message_logfile_v(priority, fmt, args);
  va_end(args);

  va_start(args, fmt);
  log_message_syslog_v(priority, fmt, args);
  va_end(args);

  if (priority < LOG_WARNING) {  // TODO: does this belong here?
    exit(EXIT_FAILURE);
  }
}

void log_message_logfile_v(int priority, const char* fmt, va_list args) {
  if (is_logfile_open && log_stream != NULL) {
    if (strcmp(log_filename, "(stdout)") == 0) {
      fprintf(log_stream, "[%s] ", log_priority_readable[priority]);

    } else {
      time_t     time_now    = time(0);
      struct tm* time_struct = localtime(&time_now);

      char date_string[32];
      char time_string[32];

      strftime(date_string, 32, "%Y-%m-%d", time_struct);
      strftime(time_string, 32, "%H:%M:%S", time_struct);

      fprintf(log_stream, "[%s] [%s] [%s] ", date_string, time_string, log_priority_readable[priority]);
    }

    vfprintf(log_stream, fmt, args);
    fprintf(log_stream, "\n");
  }
}

void log_message_syslog_v(int priority, const char* fmt, va_list args) {
  if (is_syslog_open) {
    vsyslog(priority, fmt, args);
  }
}

void log_message_logfile(int priority, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_message_logfile_v(priority, fmt, args);
  va_end(args);
}

void log_message_syslog(int priority, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_message_syslog_v(priority, fmt, args);
  va_end(args);
}
