#ifndef VOLUME_H
#define VOLUME_H

#if defined(__OpenBSD__)
#include <stdbool.h>

struct volinfo {
  bool ismute;
  float left, right;
};

char *get_volume(struct volinfo *stat);
#endif
#endif
