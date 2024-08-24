#include <stdio.h>
#include "volume.h"
#include "common.h"

#include <stdlib.h>

char *get_volume() {
  char *buffer = (char *)malloc(128);
#if defined(__OpenBSD__)
  const char *command = run_command_s("sndioctl output.level | "
                                      "awk -F '=' '{print int($2 * 100)}'");
  snprintf(buffer, 16, "VOL: %s%s", command, "% |");
#endif

  return buffer;
}

