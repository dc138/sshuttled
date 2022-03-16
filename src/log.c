#include "log.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

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
  syslog(LOG_INFO, "Started %s", identifier);

  if (filename != NULL) {
    log_stream = fopen(filename, "a+");

    if (log_stream == NULL) {
      syslog(LOG_WARNING, "Can not open log file %s, falling back to stdout, error: %s", filename, strerror(errno));

    } else {
      syslog(LOG_INFO, "Using %s as log file", filename);
    }
  }

  is_log_open = true;

  if (log_stream == NULL) {
    log_stream = stdout;
    log_message(LOG_INFO, "No log file specified, falling back to stdout\n");
  }
}

void log_close() {
  assert(is_log_open == true && "Log must be open");

  log_flush();

  if (log_stream != NULL && log_stream != stdout) {
    fclose(log_stream);
    syslog(LOG_INFO, "Closed %s", log_filename);
  }

  syslog(LOG_INFO, "Stopped %s", log_identifier);
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
    fprintf(log_stream, "[%s] ", log_priority_readable[priority]);

    va_start(args, fmt);
    vfprintf(log_stream, fmt, args);
    va_end(args);
  }

  va_start(args, fmt);  // Must call va_start once again because the call to vfprintf "consumes" the pointer
  vsyslog(priority, fmt, args);
  va_end(args);

  if (priority < LOG_WARNING) {
    exit(EXIT_FAILURE);
  }
}
