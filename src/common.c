#include <string.h>

#include "common.h"

void draw_text(Display *display, Window win, GC gc, int x, int y, const char *text) {
  XDrawString(display, win, gc, x, y, text, strlen(text));
}
