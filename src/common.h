#ifndef COMMON_H
#define COMMON_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

void draw_text(XftDraw *d, XftColor *c, XftFont *f, int x, int y, const char *t);
long long int run_command_lld(const char *command);
const char *run_command_s(const char *command);

#endif
