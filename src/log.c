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
bool        is_log_open    = false;

static FILE*       log_stream = NULL;
static const char* log_priority_readable[8] =
    {"EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void log_open(const char* identifier, const char* filename) {
  assert(identifier != NULL && "Identifier must not be null");
  assert(is_log_open == false && "Log must not be open");

  log_identifier = identifier;
  log_filename   = filename;

  openlog(identifier, LOG_PID | LOG_CONS, LOG_DAEMON);
  is_log_open = true;

  if (filename != NULL) {
    log_stream = fopen(filename, "a+");

    if (log_stream == NULL) {
      log_stream = stdout;
      log_message(LOG_INFO, "Started %s", identifier);
      log_message(
          LOG_WARNING, "Can not open log file %s, falling back to stdout, error: %s", filename, strerror(errno));

    } else {
      log_message(LOG_INFO, "Started %s", identifier);
      log_message(LOG_INFO, "Opened %s as log file", filename);
    }

  } else {
    log_stream = stdout;
    log_message(LOG_INFO, "Started %s", identifier);
    log_message(LOG_INFO, "No filename specified, falling back to stdout");
  }
}

void log_close() {
  assert(is_log_open == true && "Log must be open");

  log_message(LOG_INFO, "Closing %s", log_filename);
  log_flush();

  if (log_stream != NULL && log_stream != stdout) {
    fclose(log_stream);
    syslog(LOG_INFO, "Closed %s", log_filename);
  }

  log_message(LOG_INFO, "Stopped %s", log_identifier);

  closelog();
  is_log_open = false;
}

void log_flush() {
  assert(is_log_open == true && "Log must be open");

  if (log_stream != NULL && fflush(log_stream) != 0) {
    syslog(LOG_ERR, "Can not fflush() log stream: %s, error: %s", log_filename, strerror(errno));
  }
}

void log_message(int priority, const char* fmt, ...) {
  assert(is_log_open == true && "Log must be open");
  assert(priority >= 0 && priority < 8 && "Invalid priority value");

  va_list args;

  if (log_stream != NULL) {
    time_t     time_now    = time(0);
    struct tm* time_struct = localtime(&time_now);

    char date[32];
    char time[32];

    strftime(date, 32, "%Y-%m-%d", time_struct);
    strftime(time, 32, "%H:%M:%S", time_struct);

    fprintf(log_stream, "[%s] [%s] [%s] ", date, time, log_priority_readable[priority]);

    va_start(args, fmt);
    vfprintf(log_stream, fmt, args);
    va_end(args);

    fprintf(log_stream, "\n");
  }

  va_start(args, fmt);  // Must call va_start once again because the call to vfprintf "consumes" the pointer
  vsyslog(priority, fmt, args);
  va_end(args);

  if (priority < LOG_WARNING) {
    exit(EXIT_FAILURE);
  }
}
