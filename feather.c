#include "config.h"
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

Display *display;
int screen;

XColor getColor(char *colorString) {
  Colormap cmap = DefaultColormap(display, screen);
  XColor color;

  if (!XAllocNamedColor(display, cmap, colorString, &color, &color))
    exit(1);
  return color;
}

int main(void) {
  Window window;
  Window root;
  XEvent event;

  XftFont *font;
  XftDraw *draw;
  XRenderColor render_color;
  XftColor xft_color;

  Window foc;
  int revert_to;

  int shown = 1;

  char *status = "feather 0.1";

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  screen = DefaultScreen(display);

  XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("l")), Mod1Mask,
           DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);

  unsigned int windowWidth = DisplayWidth(display, screen);
  unsigned int windowHeight = DisplayHeight(display, screen);

  root = RootWindow(display, screen);
  XSetWindowAttributes attributes;
  attributes.override_redirect = True;
  attributes.border_pixel = getColor(borderColor).pixel;
  attributes.background_pixel = getColor(backgroundColor).pixel;
  window = XCreateWindow(
      display, root, windowWidth - width - 2 * borderSize - gap,
      windowHeight - height - 2 * borderSize - gap, width, height, borderSize,
      DefaultDepth(display, screen), CopyFromParent,
      DefaultVisual(display, screen),
      CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

  XColor color = getColor(fontColor);
  render_color.red = color.red;
  render_color.green = color.green;
  render_color.blue = color.blue;
  render_color.alpha = 0xffff;

  XftColorAllocValue(display, DefaultVisual(display, screen),
                     DefaultColormap(display, screen), &render_color,
                     &xft_color);
  font = XftFontOpenName(display, screen, fontString);

  draw = XftDrawCreate(display, window, DefaultVisual(display, screen),
                       DefaultColormap(display, screen));

  XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);
  XSelectInput(display, root, PropertyChangeMask);

  XMapWindow(display, window);

  while (1) {
    XNextEvent(display, &event);
    XGetInputFocus(display, &foc, &revert_to);

    if (event.type == Expose) {
      XftDrawStringUtf8(draw, &xft_color, font, 0, font->ascent,
                        (XftChar8 *)status, strlen(status));
    }

    if (event.type == PropertyNotify) {
      XPropertyEvent *ev = &event.xproperty;
      if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
        XClearWindow(display, window);
        XFetchName(display, root, &status);
        XftDrawStringUtf8(draw, &xft_color, font, 0, font->ascent,
                          (XftChar8 *)status, strlen(status));
      }
    }

    if (event.type == ButtonPress) {
      if (event.xbutton.button == 1 || event.xbutton.button == 3)
        XUnmapWindow(display, window);
      shown = 0;
    }

    if (event.type == KeyPress && event.xbutton.subwindow != None) {
      if (event.xkey.keycode ==
          XKeysymToKeycode(display, XStringToKeysym("l"))) {
        if (shown) {
          XUnmapWindow(display, window);
          shown = 0;
        } else {
          XMapWindow(display, window);
          shown = 1;
        }
      }
    }
  }

  XCloseDisplay(display);

  return 0;
}