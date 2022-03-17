#include "dirs.h"
#include "log.h"

#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void dirs_create(const char* filepath, int mode) {
  if (mkdir(filepath, mode) == -1 && errno != EEXIST) {
    log_message(LOG_ERR, "Error while trying to create directory %s, %s", filepath, strerror(errno));
  }
}
