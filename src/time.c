/* #include <stdio.h> */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "time.h"

char *get_current_time() {
  time_t t;
  struct tm *tm_info;
  static char buffer[9];

  time(&t);
  tm_info = localtime(&t);
  strftime(buffer, 9, "%H:%M:%S", tm_info);

  return buffer;
}
