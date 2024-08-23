#ifndef BATTERY_H
#define BATTERY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#if defined(__OpenBSD__)
#include <sys/ioctl.h>
#include <fcntl.h>
#include <machine/apmvar.h>

static struct apm_power_info powerinf;
static int apm_dev;

struct batinfo {
  bool isplugged;
  float perc;
};

char *get_battery_percent(struct batinfo *stat);
#endif
#endif
