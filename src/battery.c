#if defined(__OpenBSD__)
#include <unistd.h>

#include "battery.h"

char *get_battery_percent(struct batinfo *stat) {
  char *buffer = (char *)malloc(128);

  apm_dev = open("/dev/apm", O_RDONLY);
  if (ioctl(apm_dev, APM_IOC_GETPOWER, &powerinf) < 0) {
    free(buffer);
    return NULL;
  }
  if (APM_AC_OFF != powerinf.ac_state && APM_AC_ON != powerinf.ac_state) {
    free(buffer);
    close(apm_dev);
    return NULL;
  }

  stat->isplugged = false;
  if (powerinf.ac_state == APM_AC_ON) {
    stat->isplugged = true;
  }
  stat->perc = powerinf.battery_life;

  close(apm_dev);

  snprintf(buffer, 17, "BAT: %.2f%s | ", stat->perc, "%");
  return buffer;
}
#endif
