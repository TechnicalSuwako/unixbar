#ifndef COMMON_H
#define COMMON_H

#include <X11/Xlib.h>

void draw_text(Display *display, Window win, GC gc, int x, int y, const char *text);

#endif
