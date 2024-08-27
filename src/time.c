/* #include <stdio.h> */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "time.h"

char *get_current_date() {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  static char buffer[12];

  snprintf(
    buffer, sizeof(buffer),
    "%04d年%02d月%02d日", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday
  );
  return buffer;
}

char *get_current_time() {
  time_t t;
  struct tm *tm_info;
  static char buffer[9];

  time(&t);
  tm_info = localtime(&t);
  strftime(buffer, 9, "%H:%M:%S", tm_info);

  return buffer;
}
