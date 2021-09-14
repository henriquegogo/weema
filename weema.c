/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2021.
 * MIT License */

#include <X11/Xlib.h>
#include <stdlib.h>
#include "config.h"

Display *dpy = NULL;
XButtonEvent click_ev;
XWindowAttributes click_attr;
int scr_width, scr_height;
int top = 0, left = 0;

KeyCode up_key, down_key, left_key, right_key, r_key, t_key, l_key, b_key, w_key,
        vol_up_key, vol_down_key, f4_key, del_key, tab_key, print_key;

Window VisibleWindow(unsigned int iwin) {
    unsigned int nwins, count = 1;
    Window win, *wins;
    XQueryTree(dpy, XDefaultRootWindow(dpy), &win, &win, &wins, &nwins);
    top = 0;

    for (int i = nwins - 1; i > 0; i--) {
        XWindowAttributes wattr;
        XGetWindowAttributes(dpy, wins[i], &wattr);

        if (wattr.height <= 64 && wattr.y == 0 && wattr.map_state == IsViewable) {
            top = wattr.height;
        } else if (wins[i] != None && !wattr.override_redirect && wattr.map_state == IsViewable && iwin >= count++) {
            win = wins[i];
        }
    }

    XFree(wins);
    return win;
}

void CloseWindow(Window win) {
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, win, False, NoEventMask, &ev);
}

void SetupScreen(XWindowAttributes wattr) {
    if (wattr.x < wattr.screen->width) {
        left = 0;
        scr_width = wattr.screen->width;
        scr_height = wattr.screen->height;
    } else {
        XWindowAttributes rattr;
        XGetWindowAttributes(dpy, XDefaultRootWindow(dpy), &rattr);
        left = scr_width;
        scr_width = rattr.width - wattr.screen->width;
        scr_height = rattr.height;
    }
}

void ResizeUp(Window win, XWindowAttributes wattr) {
    if (wattr.height <= 2 * scr_height / 4 - top) {
        XResizeWindow(dpy, win, wattr.width, scr_height / 4 - top);
    } else if (wattr.height <= 3 * scr_height / 4 - top) {
        XResizeWindow(dpy, win, wattr.width, 2 * scr_height / 4 - top);
    } else if (wattr.height <= scr_height - top) {
        XResizeWindow(dpy, win, wattr.width, 3 * scr_height / 4 - top);
    }
}

void ResizeDown(Window win, XWindowAttributes wattr) {
    if (wattr.height < 2 * scr_height / 4 - top) {
        XResizeWindow(dpy, win, wattr.width, 2 * scr_height / 4 - top);
    } else if (wattr.height < 3 * scr_height / 4 - top) {
        XResizeWindow(dpy, win, wattr.width, 3 * scr_height / 4 - top);
    } else {
        XMoveResizeWindow(dpy, win, wattr.x, top, wattr.width, scr_height - top);
    }
}

void ResizeLeft(Window win, XWindowAttributes wattr) {
    if (wattr.width <= 2 * scr_width / 4) {
        XResizeWindow(dpy, win, scr_width / 4, wattr.height);
    } else if (wattr.width <= 3 * scr_width / 4) {
        XResizeWindow(dpy, win, 2 * scr_width / 4, wattr.height);
    } else if (wattr.width <= scr_width) {
        XResizeWindow(dpy, win, 3 * scr_width / 4, wattr.height);
    }
}

void ResizeRight(Window win, XWindowAttributes wattr) {
    if (wattr.width < 2 * scr_width / 4) {
        XResizeWindow(dpy, win, 2 * scr_width / 4, wattr.height);
    } else if (wattr.width < 3 * scr_width / 4) {
        XResizeWindow(dpy, win, 3 * scr_width / 4, wattr.height);
    } else {
        XMoveResizeWindow(dpy, win, left, wattr.y, scr_width, wattr.height);
    }
}

void PositionUp(Window win, XWindowAttributes wattr) {
    if (wattr.y == top) {
        XMoveResizeWindow(dpy, win, left, top, scr_width, scr_height - top);
    } else {
        XMoveWindow(dpy, win, wattr.x, top);
    }
}

void PositionDown(Window win, XWindowAttributes wattr) {
    if (wattr.width == scr_width && wattr.height == scr_height - top) {
        XMoveResizeWindow(dpy, win, scr_width / 3 / 2 + left, scr_height / 3 / 2,
                scr_width / 3 * 2, scr_height / 3 * 2);
    } else {
        XMoveWindow(dpy, win, wattr.x, scr_height - wattr.height);
    }
}

void PositionLeft(Window win, XWindowAttributes wattr) {
    if (wattr.x == left) {
        XMoveResizeWindow(dpy, win, left, top, wattr.width, scr_height - top);
    } else {
        XMoveWindow(dpy, win, left, wattr.y);
    }
}

void PositionRight(Window win, XWindowAttributes wattr) {
    int right_pos = scr_width - wattr.width + left;

    if (wattr.x == right_pos) {
        XMoveResizeWindow(dpy, win, right_pos, top, wattr.width, scr_height - top);
    } else {
        XMoveWindow(dpy, win, scr_width - wattr.width + left, wattr.y);
    }
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XWindowAttributes wattr;
    XGetWindowAttributes(dpy, win, &wattr);
    SetupScreen(wattr);

    if (keycode == up_key && modifiers & ShiftMask) {
        ResizeUp(win, wattr);
    } else if (keycode == down_key && modifiers & ShiftMask) {
        ResizeDown(win, wattr);
    } else if (keycode == left_key && modifiers & ShiftMask) {
        ResizeLeft(win, wattr);
    } else if (keycode == right_key && modifiers & ShiftMask) {
        ResizeRight(win, wattr);
    } else if (keycode == up_key && modifiers & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y - scr_height / 4);
    } else if (keycode == down_key && modifiers & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y + scr_height / 4);
    } else if (keycode == left_key && modifiers & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x - scr_width / 4, wattr.y);
    } else if (keycode == right_key && modifiers & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x + scr_width / 4, wattr.y);
    } else if (keycode == up_key) {
        PositionUp(win, wattr);
    } else if (keycode == down_key) {
        PositionDown(win, wattr);
    } else if (keycode == left_key) {
        PositionLeft(win, wattr);
    } else if (keycode == right_key) {
        PositionRight(win, wattr);
    }

    XRaiseWindow(dpy, win);
}

void HandleNewWindow(Window win) {
    XWindowAttributes wattr;
    XGetWindowAttributes(dpy, win, &wattr);

    if (win != None && !wattr.override_redirect && wattr.map_state == IsViewable) {
        XSetWindowBorderWidth(dpy, win, 1);
        XSetWindowBorder(dpy, win, 0);
        XRaiseWindow(dpy, win);
        XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime); 
        XSelectInput(dpy, win, FocusChangeMask);
        XMoveWindow(dpy, VisibleWindow(1), wattr.x, top);
    }
}

void HandleClick(XButtonEvent ev) {
    XGrabPointer(dpy, ev.subwindow, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(dpy, ev.subwindow, &click_attr);
    click_ev = ev;
}

void HandleMotion(XEvent ev) {
    int xdiff = ev.xbutton.x_root - click_ev.x_root;
    int ydiff = ev.xbutton.y_root - click_ev.y_root;

    if (click_ev.button == Button1) {
        XMoveWindow(dpy, ev.xmotion.window, click_attr.x + xdiff, click_attr.y + ydiff);
    } else if (click_ev.button == Button3) {
        XResizeWindow(dpy, ev.xmotion.window, abs(click_attr.width + xdiff) + 1, abs(click_attr.height + ydiff) + 1);
    }
}

void GrabKey(int keycode, unsigned int modifiers) {
    XGrabKey(dpy, keycode, modifiers, XDefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
}

int GetKeycode(const char *key) {
    return XKeysymToKeycode(dpy, XStringToKeysym(key));
}

void SetupGrab() {
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };

    for (int i = 0; i < 8; i++) {
        GrabKey(vol_up_key   = GetKeycode("XF86AudioRaiseVolume"), modifiers[i]);
        GrabKey(vol_down_key = GetKeycode("XF86AudioLowerVolume"), modifiers[i]);
        GrabKey(r_key     = GetKeycode("r"),      Mod4Mask|modifiers[i]);
        GrabKey(t_key     = GetKeycode("t"),      Mod4Mask|modifiers[i]);
        GrabKey(l_key     = GetKeycode("l"),      Mod4Mask|modifiers[i]);
        GrabKey(b_key     = GetKeycode("b"),      Mod4Mask|modifiers[i]);
        GrabKey(print_key = GetKeycode("Print"),  modifiers[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),    Mod1Mask|modifiers[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),    ShiftMask|Mod1Mask|modifiers[i]);
        GrabKey(f4_key    = GetKeycode("F4"),     Mod1Mask|modifiers[i]);
        GrabKey(w_key     = GetKeycode("w"),      ControlMask|ShiftMask|modifiers[i]);
        GrabKey(del_key   = GetKeycode("Delete"), ControlMask|Mod1Mask|modifiers[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod4Mask|modifiers[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod4Mask|modifiers[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod4Mask|modifiers[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod4Mask|modifiers[i]);
        GrabKey(up_key    = GetKeycode("Up"),     ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(down_key  = GetKeycode("Down"),   ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(left_key  = GetKeycode("Left"),   ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(right_key = GetKeycode("Right"),  ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod1Mask|Mod4Mask|modifiers[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod1Mask|Mod4Mask|modifiers[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod1Mask|Mod4Mask|modifiers[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod1Mask|Mod4Mask|modifiers[i]);
        XGrabButton(dpy, AnyButton, Mod1Mask|modifiers[i], XDefaultRootWindow(dpy), True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);
    }
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(dpy, &ev);

    if (ev.type == KeyPress && ev.xkey.keycode == r_key) {
        system(CMD_LAUNCHER);
    } else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
        system(CMD_TERMINAL);
    } else if (ev.type == KeyPress && ev.xkey.keycode == b_key) {
        system(CMD_BROWSER);
    } else if (ev.type == KeyPress && ev.xkey.keycode == l_key) {
        system(CMD_LOCK);
    } else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
        system(CMD_VOLUMEUP);
    } else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
        system(CMD_VOLUMEDOWN);
    } else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
        system(CMD_PRINTSCREEN);
    } else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        XRaiseWindow(dpy, ev.xkey.state & ShiftMask ? VisibleWindow(999999999) : VisibleWindow(2));
    } else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(dpy);
    } else if (ev.type == KeyPress && (ev.xkey.keycode == f4_key || ev.xkey.keycode == w_key)) {
        CloseWindow(VisibleWindow(1));
    } else if (ev.type == KeyPress) {
        HandleWindowPosition(VisibleWindow(1), ev.xkey.keycode, ev.xkey.state);
    } else if (ev.type == ButtonPress && ev.xbutton.window != XDefaultRootWindow(dpy)) {
        XRaiseWindow(dpy, ev.xbutton.window);
    } else if (ev.type == ButtonPress && ev.xbutton.window == XDefaultRootWindow(dpy)
            && ev.xbutton.subwindow != None && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
        XRaiseWindow(dpy, ev.xbutton.subwindow);
        HandleClick(ev.xbutton);
    } else if (ev.type == ButtonRelease) {
        XUngrabPointer(dpy, CurrentTime);
    } else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    } else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    } else if (ev.type == UnmapNotify) {
        XSetInputFocus(dpy, VisibleWindow(1), RevertToPointerRoot, CurrentTime);
    } else if (ev.type == FocusIn) {
        XUngrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
    } else if (ev.type == FocusOut) {
        XGrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window, True, ButtonPressMask,
                GrabModeSync, GrabModeSync, None, None);
    } else if (ev.type == ConfigureNotify) {
        XSetInputFocus(dpy, ev.xconfigure.window, RevertToPointerRoot, CurrentTime); 
    }
}

int ErrorHandler() {
    return 0;
}

int main() {
    if (!(dpy = XOpenDisplay(NULL))) return 1;

    XSetErrorHandler(ErrorHandler);
    XSelectInput(dpy, XDefaultRootWindow(dpy), SubstructureNotifyMask);
    SetupGrab(); 
    system("xsetroot -cursor_name arrow -solid \"#030609\"");
    system(CMD_INIT);

    for(;;) InterceptEvents();
}
