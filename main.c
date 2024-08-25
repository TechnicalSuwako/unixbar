#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "src/common.h"
#include "src/time.h"
#if defined(__OpenBSD__)
#include "src/battery.h"
#include "src/volume.h"
#endif

#define BAR_HEIGHT 36
#define FONT "Noto Sans CJK JP-10"

const char *sofname = "unixbar";
const char *version = "0.1.0";

int get_current_workspace(Display *dp, Window root) {
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop = NULL;
  Atom _NET_CURRENT_DESKTOP = XInternAtom(dp, "_NET_CURRENT_DESKTOP", False);
  int workspace = 0;

  if (XGetWindowProperty(
    dp,
    root,
    _NET_CURRENT_DESKTOP,
    0,
    1,
    False,
    XA_CARDINAL, 
    &actual_type,
    &actual_format,
    &nitems,
    &bytes_after,
    &prop
  ) == Success) {
    if (prop != NULL) {
      workspace = (int) *prop;
      XFree(prop);
    }
  }
  
  return workspace;
}

Window *get_open_windows(Display *dp, Window root, unsigned long *nwindows) {
  Atom _NET_CLIENT_LIST = XInternAtom(dp, "_NET_CLIENT_LIST", False);
  Atom _NET_WM_DESKTOP = XInternAtom(dp, "_NET_WM_DESKTOP", False);
  
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop = NULL;

  if (XGetWindowProperty(
    dp,
    root,
    _NET_CLIENT_LIST,
    0,
    ~0L,
    False,
    XA_WINDOW,
    &actual_type,
    &actual_format,
    &nitems,
    &bytes_after,
    &prop
  ) != Success) {
    return NULL;
  }

  Window *windows = (Window *)prop;
  *nwindows = nitems;
  unsigned long current_workspace =
    (unsigned long)get_current_workspace(dp, root);

  unsigned long j = 0;
  for (unsigned long i = 0; i < nitems; i++) {
    unsigned long desktop;
    Atom type;
    int format;
    unsigned long nitems2, bytes_after2;
    unsigned char *prop2 = NULL;
    
    if (XGetWindowProperty(
      dp,
      windows[i],
      _NET_WM_DESKTOP,
      0,
      1,
      False,
      XA_CARDINAL, 
      &type,
      &format,
      &nitems2,
      &bytes_after2,
      &prop2
    ) == Success) {
      if (prop2 != NULL) {
        desktop = *(unsigned long *)prop2;
        XFree(prop2);

        if (desktop == current_workspace) {
          windows[j++] = windows[i];
        }
      }
    }
  }

  *nwindows = j;
  return windows;
}

int main() {
  Display *dp;
  int screen;
  Window root, win;
  XEvent ev;
  GC gc;
  XGCValues values;
  int width, height;
  Window *windows;
  unsigned long nwindows;

  dp = XOpenDisplay(NULL);
  if (dp == NULL) {
    fprintf(stderr, "画面を開けられません\n");
    return 1;
  }

  screen = DefaultScreen(dp);
  root = RootWindow(dp, screen);

  width = DisplayWidth(dp, screen);
  height = BAR_HEIGHT;

  win = XCreateSimpleWindow(
    dp,
    root,
    0,
    0,
    width,
    height,
    0,
    BlackPixel(dp, screen),
    BlackPixel(dp, screen)
  );

  XSetWindowAttributes attrs;
  attrs.override_redirect = True;
  XChangeWindowAttributes(dp, win, CWOverrideRedirect, &attrs);

  XSelectInput(dp, win, ExposureMask | StructureNotifyMask);
  XMapWindow(dp, win);

  int depth = DefaultDepth(dp, screen);
  Pixmap buf = XCreatePixmap(dp, win, width, height, depth);
  XftDraw *xft_draw = XftDrawCreate(
    dp, buf, DefaultVisual(dp, screen), DefaultColormap(dp, screen)
  );
  XftFont *xft_font = XftFontOpenName(dp, screen, FONT);
  XftColor xft_color;
  XRenderColor render_color = {0xffEA, 0x0079, 0xffD8, 0xffff};
  XftColorAllocValue(
    dp, DefaultVisual(dp, screen), DefaultColormap(dp, screen),
    &render_color, &xft_color
  );

  gc = XCreateGC(dp, win, 0, &values);

  while (1) {
    while (XPending(dp)) {
      XNextEvent(dp, &ev);
      if (ev.type == Expose) {
        break;
      } else if (ev.type == ButtonPress) {
        XButtonEvent *bev = (XButtonEvent *)&ev;

        if (nwindows > 0) {
          unsigned long window_index = bev->x / (width / nwindows);
          if (window_index < nwindows) {
            XRaiseWindow(dp, windows[window_index]);
          }
        }
      }
    }

    int workspace = get_current_workspace(dp, root);

    windows = get_open_windows(dp, root, &nwindows);
    if (windows == NULL) {
      fprintf(stderr, "ウィンドウを開くに失敗\n");
      continue;
    }

    XSetForeground(dp, gc, 0x120f12);
    XFillRectangle(dp, buf, gc, 0, 0, width, height);

    XftDrawChange(xft_draw, buf);

    char workspace_str[10];
    snprintf(workspace_str, 10, "W%d", workspace);
    draw_text(xft_draw, &xft_color, xft_font, 10, height - 10, workspace_str);

    int xpos = 50;
    if (nwindows > 0) {
      int window_width = 200;
      for (unsigned long i = 0; i < nwindows; i++) {
        char *name;
        XFetchName(dp, windows[i], &name);
        if (!name) name = "不明";
        if (nwindows < 7) {
          draw_text(xft_draw, &xft_color, xft_font, xpos, height - 10, name);
          xpos += window_width;
        } else {
          draw_text(xft_draw, &xft_color, xft_font, xpos, height - 10, "複数");
          xpos = 50;
        }
      }
    }

    char *time_str = get_current_time();
    if (time_str)
      draw_text(xft_draw, &xft_color, xft_font, width - 80, height - 10, time_str);
#if defined(__OpenBSD__)
    struct batinfo bstat;
    char *batperc = get_battery_percent(&bstat);
    if (batperc) {
      draw_text(xft_draw, &xft_color, xft_font, width - 180, height - 10, batperc);
      free(batperc);
    }

    char *volperc = get_volume();
    if (volperc) {
      int volw = width - 290;
      if (strncmp(volperc, "VOL: 100% |", 11) != 0) volw = width - 280;
      draw_text(xft_draw, &xft_color, xft_font, volw, height - 10, volperc);
      free(volperc);
    }
#endif

    XCopyArea(dp, buf, win, DefaultGC(dp, screen), 0, 0, width, height, 0, 0);
    XFlush(dp);
    sleep(1);
  }

  XftDrawDestroy(xft_draw);
  XftFontClose(dp, xft_font);
  XftColorFree(
    dp, DefaultVisual(dp, screen),
    DefaultColormap(dp, screen), &xft_color
  );
  XFreePixmap(dp, buf);
  XCloseDisplay(dp);

  return 0;
}
