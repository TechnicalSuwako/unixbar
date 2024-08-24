#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

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
#define FONT "-*-fixed-*-*-*-*-18-*-*-*-*-*-*-*"

const char *sofname = "unixbar";
const char *version = "0.1.0";

int get_current_workspace(Display *display, Window root) {
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop = NULL;
  Atom _NET_CURRENT_DESKTOP = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
  int workspace = 0;

  if (XGetWindowProperty(
    display,
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

Window *get_open_windows(Display *display, Window root, unsigned long *nwindows) {
  Atom _NET_CLIENT_LIST = XInternAtom(display, "_NET_CLIENT_LIST", False);
  Atom _NET_WM_DESKTOP = XInternAtom(display, "_NET_WM_DESKTOP", False);
  
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop = NULL;

  if (XGetWindowProperty(
    display,
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
    (unsigned long)get_current_workspace(display, root);

  unsigned long j = 0;
  for (unsigned long i = 0; i < nitems; i++) {
    unsigned long desktop;
    Atom type;
    int format;
    unsigned long nitems2, bytes_after2;
    unsigned char *prop2 = NULL;
    
    if (XGetWindowProperty(
      display,
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
  Display *display;
  int screen;
  Window root, win;
  XEvent ev;
  GC gc;
  XGCValues values;
  int width, height;
  Window *windows;
  unsigned long nwindows;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "画面を開けられません\n");
    return 1;
  }

  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

  width = DisplayWidth(display, screen);
  height = BAR_HEIGHT;

  win = XCreateSimpleWindow(
    display,
    root,
    0,
    0,
    width,
    height,
    0,
    BlackPixel(display, screen),
    BlackPixel(display, screen)
  );

  XSetWindowAttributes attrs;
  attrs.override_redirect = True;
  XChangeWindowAttributes(display, win, CWOverrideRedirect, &attrs);

  XSelectInput(display, win, ExposureMask | StructureNotifyMask);
  XMapWindow(display, win);

  gc = XCreateGC(display, win, 0, &values);
  XSetFont(display, gc, XLoadFont(display, FONT));

  XColor magenta;
  Colormap colormap = DefaultColormap(display, screen);
  XAllocNamedColor(display, colormap, "magenta", &magenta, &magenta);
  XSetForeground(display, gc, magenta.pixel);

  while (1) {
    while (XPending(display)) {
      XNextEvent(display, &ev);
      if (ev.type == Expose) {
        break;
      } else if (ev.type == ButtonPress) {
        XButtonEvent *bev = (XButtonEvent *)&ev;

        if (nwindows > 0) {
          unsigned long window_index = bev->x / (width / nwindows);
          if (window_index < nwindows) {
            XRaiseWindow(display, windows[window_index]);
          }
        }
      }
    }

    int workspace = get_current_workspace(display, root);

    windows = get_open_windows(display, root, &nwindows);
    if (windows == NULL) {
      fprintf(stderr, "ウィンドウを開くに失敗\n");
      continue;
    }
    XClearWindow(display, win);

    char workspace_str[10];
    snprintf(workspace_str, 10, "W%d", workspace);
    draw_text(display, win, gc, 10, height - 10, workspace_str);

    int xpos = 50;
    if (nwindows > 0) {
      /* int window_width = (width - 200) / nwindows; */
      int window_width = 200;
      for (unsigned long i = 0; i < nwindows; i++) {
        char *name;
        XFetchName(display, windows[i], &name);
        if (!name) name = "unknown";
        if (nwindows < 7) {
          draw_text(display, win, gc, xpos, height - 10, name);
          xpos += window_width;
        } else {
          draw_text(display, win, gc, xpos, height - 10, "many");
          xpos = 50;
        }
      }
    }

    char *time_str = get_current_time();
    if (time_str) draw_text(display, win, gc, width - 80, height - 10, time_str);
#if defined(__OpenBSD__)
    struct batinfo bstat;
    char *battery_percent = get_battery_percent(&bstat);
    if (battery_percent) {
      draw_text(display, win, gc, width - 180, height - 10, battery_percent);
      free(battery_percent);
    }

    char *volume_percent = get_volume();
    if (volume_percent) {
      draw_text(display, win, gc, width - 280, height - 10, volume_percent);
      free(volume_percent);
    }
#endif

    XFlush(display);
    sleep(1);
  }

  XCloseDisplay(display);
  return 0;
}
