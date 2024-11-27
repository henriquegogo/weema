/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2024.
 * MIT License */

#include <X11/Xlib.h>
#include <stdlib.h>
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

Display *dpy = NULL;
XButtonEvent click_ev;
XWindowAttributes click_attr;
KeyCode up_key, down_key, left_key, right_key, w_key, f4_key, del_key, tab_key;

Window Clients(unsigned int iwin, Bool refresh) {
    unsigned int nwins, count = 1;
    XWindowAttributes wattr;
    Window win, *wins;
    XQueryTree(dpy, XDefaultRootWindow(dpy), &win, &win, &wins, &nwins);

    if (refresh) {
        XDeleteProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "_NET_CLIENT_LIST", False));
    }

    for (int i = nwins - 1; i > 0; i--) {
        XGetWindowAttributes(dpy, wins[i], &wattr);

        if (refresh && wins[i] != None && !wattr.override_redirect && wattr.map_state == IsViewable) {
            XChangeProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "_NET_CLIENT_LIST", False), 33, 32,
                    PropModeAppend, (unsigned char *) &(wins[i]), 1);
        }

        if (wins[i] != None && !wattr.override_redirect && wattr.map_state == IsViewable && iwin >= count++) {
            win = wins[i];
        }
    }

    XFree(wins);
    return win;
}

void SendEvent(Window win, const char *atom_name) {
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(dpy, atom_name, False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, win, False, NoEventMask, &ev);
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int mods) {
    XWindowAttributes wattr, rattr;
    XGetWindowAttributes(dpy, win, &wattr);
    XGetWindowAttributes(dpy, XDefaultRootWindow(dpy), &rattr);
    int top = 0;
    int left = 0;
    int scr_width = wattr.screen->width;
    int scr_height = wattr.screen->height;
    int scnd_scr_width = rattr.width - scr_width;
    int scnd_scr_height = rattr.height - scr_height;

    if (wattr.x < 0) {
        left = scnd_scr_width * -1;
    } else if (wattr.x >= scr_width) {
        left = scnd_scr_width;
    } else if (wattr.y < 0) {
        top = scnd_scr_height * -1;
    } else if (wattr.y >= scr_height) {
        top = scnd_scr_height;
    }

    scr_width = scr_width - 2;   // Borders
    scr_height = scr_height - 2; // Borders

    if (keycode == up_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MAX(wattr.height - scr_height / 4, scr_height / 4));
    } else if (keycode == down_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MIN(wattr.height + scr_height / 4, scr_height));
    } else if (keycode == left_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MAX(wattr.width - scr_width / 6, scr_width / 6), wattr.height);
    } else if (keycode == right_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MIN(wattr.width + scr_width / 6, scr_width), wattr.height);
    } else if (keycode == up_key && mods & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y - scr_height / 8);
    } else if (keycode == down_key && mods & Mod1Mask && wattr.width == scr_width && wattr.height == scr_height) {
        XMoveWindow(dpy, win, wattr.x, wattr.height + 2);
    } else if (keycode == down_key && mods & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y + scr_height / 8);
    } else if (keycode == left_key && mods & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x - scr_width / 12, wattr.y);
    } else if (keycode == right_key && mods & Mod1Mask && wattr.width == scr_width && wattr.height == scr_height) {
        XMoveWindow(dpy, win, wattr.width + 2, wattr.y);
    } else if (keycode == right_key && mods & Mod1Mask) {
        XMoveWindow(dpy, win, wattr.x + scr_width / 12, wattr.y);
    } else if (keycode == up_key && wattr.y == top) {
        XMoveResizeWindow(dpy, win, left, top, scr_width, scr_height);
    } else if (keycode == up_key) {
        XMoveWindow(dpy, win, wattr.x, top);
    } else if (keycode == down_key && wattr.width == scr_width && wattr.height == scr_height) {
        XMoveResizeWindow(dpy, win, scr_width / 20 + left, scr_height / 20 + top, scr_width * 0.9, scr_height * 0.9);
    } else if (keycode == down_key) {
        XMoveWindow(dpy, win, wattr.x, scr_height - wattr.height + top);
    } else if (keycode == left_key && wattr.x == left) {
        XMoveResizeWindow(dpy, win, left, top, wattr.width, scr_height);
    } else if (keycode == left_key) {
        XMoveWindow(dpy, win, left, wattr.y);
    } else if (keycode == right_key && wattr.x == scr_width - wattr.width + left) {
        XMoveResizeWindow(dpy, win, wattr.x, top, wattr.width, scr_height);
    } else if (keycode == right_key) {
        XMoveWindow(dpy, win, scr_width - wattr.width + left, wattr.y);
    }

    XRaiseWindow(dpy, win);
}

void HandleNewWindow(Window win) {
    XWindowAttributes wattr;
    XGetWindowAttributes(dpy, win, &wattr);

    if (win != None && !wattr.override_redirect && wattr.map_state == IsViewable) {
        XSelectInput(dpy, win, FocusChangeMask);
        XSetWindowBorderWidth(dpy, win, 1);
        XSetWindowBorder(dpy, win, 0);
        XRaiseWindow(dpy, win);
        XMoveWindow(dpy, Clients(1, True), wattr.x, 0);
    }
}

void HandleClick(XButtonEvent ev, Window win) {
    XGrabPointer(dpy, win, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(dpy, win, &click_attr);
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

void GrabKey(int keycode, unsigned int mods) {
    XGrabKey(dpy, keycode, mods, XDefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
}

KeyCode GetKeycode(const char *key) {
    return XKeysymToKeycode(dpy, XStringToKeysym(key));
}

void SetupGrab() {
    unsigned int mods[8] = { None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };

    for (int i = 0; i < 8; i++) {
        for (unsigned int ikey = 0; ikey < sizeof(CMD_KEYS) / sizeof(CMD_KEYS[0]); ikey++) {
            GrabKey(GetKeycode(CMD_KEYS[ikey][0]), CMD_KEYS[ikey][0][3] ? mods[i] : Mod4Mask|mods[i]);
        }
        GrabKey(tab_key   = GetKeycode("Tab"),    Mod1Mask|mods[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),    Mod4Mask|mods[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),    ShiftMask|Mod1Mask|mods[i]);
        GrabKey(f4_key    = GetKeycode("F4"),     Mod1Mask|mods[i]);
        GrabKey(w_key     = GetKeycode("w"),      ControlMask|ShiftMask|mods[i]);
        GrabKey(del_key   = GetKeycode("Delete"), ControlMask|Mod1Mask|mods[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod4Mask|mods[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod4Mask|mods[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod4Mask|mods[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod4Mask|mods[i]);
        GrabKey(up_key    = GetKeycode("Up"),     ShiftMask|Mod4Mask|mods[i]);
        GrabKey(down_key  = GetKeycode("Down"),   ShiftMask|Mod4Mask|mods[i]);
        GrabKey(left_key  = GetKeycode("Left"),   ShiftMask|Mod4Mask|mods[i]);
        GrabKey(right_key = GetKeycode("Right"),  ShiftMask|Mod4Mask|mods[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod1Mask|Mod4Mask|mods[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod1Mask|Mod4Mask|mods[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod1Mask|Mod4Mask|mods[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod1Mask|Mod4Mask|mods[i]);
        XGrabButton(dpy, AnyButton, Mod1Mask|mods[i], XDefaultRootWindow(dpy), True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);
    }
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(dpy, &ev);

    for (unsigned int i = 0; i < sizeof(CMD_KEYS) / sizeof(CMD_KEYS[0]); i++) {
        if (ev.type == KeyPress && ev.xkey.keycode == GetKeycode(CMD_KEYS[i][0])) {
            system(CMD_KEYS[i][1]);
        }
    }
    if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        XRaiseWindow(dpy, ev.xkey.state & (Mod4Mask|ShiftMask) ? Clients(999999999, False) : Clients(2, False));
    } else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(dpy);
    } else if (ev.type == KeyPress && (ev.xkey.keycode == f4_key || ev.xkey.keycode == w_key)) {
        SendEvent(Clients(1, False), "WM_DELETE_WINDOW");
    } else if (ev.type == KeyPress) {
        HandleWindowPosition(Clients(1, False), ev.xkey.keycode, ev.xkey.state);
    } else if (ev.type == ButtonPress && ev.xbutton.window != XDefaultRootWindow(dpy)) {
        XRaiseWindow(dpy, ev.xbutton.window);
        if (ev.xbutton.x <= 5 || ev.xbutton.y <= 5) HandleClick(ev.xbutton, ev.xbutton.window);
    } else if (ev.type == ButtonPress && ev.xbutton.window == XDefaultRootWindow(dpy)
            && ev.xbutton.subwindow != None && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
        XRaiseWindow(dpy, ev.xbutton.subwindow);
        HandleClick(ev.xbutton, ev.xbutton.subwindow);
    } else if (ev.type == ButtonRelease) {
        XUngrabPointer(dpy, CurrentTime);
    } else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    } else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    } else if (ev.type == UnmapNotify) {
        XSetInputFocus(dpy, Clients(1, True), RevertToPointerRoot, CurrentTime);
    } else if (ev.type == FocusIn) {
        XSetWindowBorder(dpy, ev.xfocus.window, WhitePixel(dpy, 0));
        XUngrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        XChangeProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False), 33, 32,
                PropModeReplace, (unsigned char *) &(ev.xfocus.window), 1);
    } else if (ev.type == FocusOut) {
        XSetWindowBorder(dpy, ev.xfocus.window, BlackPixel(dpy, 0));
        if (ev.xfocus.window != Clients(1, False)) {
            XGrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window, True, ButtonPressMask,
                    GrabModeSync, GrabModeSync, None, None);
        }
    } else if (ev.type == ConfigureNotify) {
        XSetInputFocus(dpy, ev.xconfigure.window, RevertToPointerRoot, CurrentTime); 
    } else if (ev.xclient.message_type == XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False)) {
        XSetInputFocus(dpy, ev.xclient.window, RevertToPointerRoot, CurrentTime);
    }
}

int ErrorHandler() {
    return 0;
}

int main() {
    if (!(dpy = XOpenDisplay(NULL))) return 1;

    XSetErrorHandler(ErrorHandler);
    XSelectInput(dpy, XDefaultRootWindow(dpy), SubstructureNotifyMask);
    XDefineCursor(dpy, XDefaultRootWindow(dpy), XCreateFontCursor(dpy, 2));
    SetupGrab(); 

    for (unsigned int i = 0; i < sizeof(STARTUP_CMDS) / sizeof(STARTUP_CMDS[0]); i++) {
        system(STARTUP_CMDS[i]);
    }

    for(;;) InterceptEvents();
}
