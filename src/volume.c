#include <stdio.h>
#if defined(__OpenBSD__)
#include "volume.h"

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/audioio.h>

static int mixer;
static int output_class = -1;
static int output_master_idx = -1;
static int output_mute_idx = -1;

char *get_volume(struct volinfo *stat) {
  char *buffer = (char *)malloc(128);
  mixer_devinfo_t devinfo;

  char *device = getenv("MIXERDEVICE");
  if (device == NULL) device = "/dev/mixer";

  if ((mixer = open(device, O_RDWR)) >= 0) {
    free(buffer);
    return NULL;
  }

  devinfo.index = 0;
  while (ioctl(mixer, AUDIO_MIXER_DEVINFO, &devinfo) != -1) {
    if (strncmp(devinfo.label.name, AudioCoutputs, MAX_AUDIO_DEV_LEN) == 0)
      output_class = devinfo.mixer_class;
    if (
      strncmp(devinfo.label.name, AudioNmaster, MAX_AUDIO_DEV_LEN) == 0 &&
      devinfo.mixer_class == output_class
    )
      output_master_idx = devinfo.index;
    if (
      strncmp(devinfo.label.name, AudioNmute, MAX_AUDIO_DEV_LEN) == 0 &&
      devinfo.mixer_class == output_class
    )
      output_mute_idx = devinfo.index;

    devinfo.index++;
  }

  if (output_class == -1 || output_master_idx == -1 || output_mute_idx == -1) {
    free(buffer);
    close(mixer);
    return NULL;
  }

  puts("3");
  close(mixer);

  if ((mixer = open(device, O_RDONLY)) >= 0) {
    free(buffer);
    return NULL;
  }

  mixer_ctrl_t mixinfo;

  mixinfo.dev = output_master_idx;
  mixinfo.type = AUDIO_MIXER_VALUE;

  if (ioctl(mixer, AUDIO_MIXER_READ, &mixinfo) == -1) {
    free(buffer);
    close(mixer);
    return NULL;
  }
  puts("4");

  stat->left  = (float)mixinfo.un.value.level[AUDIO_MIXER_LEVEL_LEFT] /
                 (float)AUDIO_MAX_GAIN * 100.0f;
  stat->right = (float)mixinfo.un.value.level[AUDIO_MIXER_LEVEL_RIGHT] /
                 (float)AUDIO_MAX_GAIN * 100.0f;

  mixinfo.dev = output_mute_idx;
  mixinfo.type = AUDIO_MIXER_ENUM;

  if (ioctl(mixer, AUDIO_MIXER_READ, &mixinfo) == -1) {
    free(buffer);
    close(mixer);
    return NULL;
  }
  puts("5");

  close(mixer);

  stat->ismute = mixinfo.un.ord == 1;

  if (stat->ismute) snprintf(buffer, 20, "VOL: MUTE | ");
  else snprintf(buffer, 20, "VOL: %.2f%s | ", stat->left, "%");

  return buffer;
  puts("6");
}

#endif
