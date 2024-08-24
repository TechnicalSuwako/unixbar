#ifndef COMMON_H
#define COMMON_H

#include <X11/Xlib.h>

void draw_text(Display *display, Window win, GC gc, int x, int y, const char *text);
long long int run_command_lld(const char *command);
const char *run_command_s(const char *command);

#endif
