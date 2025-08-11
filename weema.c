/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2025.
 * MIT License */

#include <X11/Xlib.h>
#include <stdlib.h>
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define NUMCODE(x) (x + 9)

Display *dpy = NULL;
XButtonEvent click_ev;
XWindowAttributes last_attr;
KeyCode enter_key, up_key, down_key, left_key, right_key, w_key, f4_key, del_key, tab_key;
Window owins[512];
int panelheight = 0;

Window Clients(unsigned int iwin, Bool refresh) {
    unsigned int nwins, count = 1;
    XWindowAttributes wattr;
    Window win, *wins;
    XQueryTree(dpy, XDefaultRootWindow(dpy), &win, &win, &wins, &nwins);

    if (refresh) XDeleteProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "_NET_CLIENT_LIST", False));

    for (int i = nwins - 1; i > 0; i--) {
        XGetWindowAttributes(dpy, wins[i], &wattr);

        if (wins[i] != None && !wattr.override_redirect && wattr.map_state == IsViewable && wattr.height > 100) {
            if (iwin >= count++) win = wins[i];
            if (refresh) XChangeProperty(dpy, XDefaultRootWindow(dpy), XInternAtom(dpy, "_NET_CLIENT_LIST", False),
                    33, 32, PropModeAppend, (unsigned char *) &(wins[i]), 1);
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
    int borders = 2;
    int top = 0 + MARGIN, left = 0 + MARGIN;
    int scr_width = wattr.screen->width - borders - MARGIN * 2;
    int scr_height = wattr.screen->height - borders - MARGIN * 2;
    int scnd_scr_width = rattr.width - scr_width - MARGIN * 2;
    int scnd_scr_height = rattr.height - scr_height - MARGIN * 2;

    if (wattr.x >= 0 && wattr.x < scr_width && wattr.y >= 0 && wattr.y < scr_height) {
       top = top + panelheight;
       scr_height = scr_height - panelheight;
    } else if (wattr.x < 0) left = scnd_scr_width * -1 + MARGIN;
    else if (wattr.x >= scr_width) left = scnd_scr_width + MARGIN;
    else if (wattr.y < 0) top = scnd_scr_height * -1 + MARGIN;
    else if (wattr.y >= scr_height) top = scnd_scr_height + MARGIN;

    if (keycode == enter_key && wattr.x != left - MARGIN && wattr.y != top - MARGIN) {
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left - MARGIN, 0, wattr.screen->width, wattr.screen->height);
    } else if (keycode == up_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MAX(wattr.height - scr_height / 4, scr_height / 4) - MARGIN / 3);
    } else if (keycode == down_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MIN(wattr.height + scr_height / 4, scr_height));
    } else if (keycode == left_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MAX(wattr.width - scr_width / 6, scr_width / 6) - MARGIN / 3, wattr.height);
    } else if (keycode == right_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MIN(wattr.width + scr_width / 6, scr_width), wattr.height);
    } else if (keycode == up_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y - scr_height / 8);
    } else if (keycode == down_key && mods & ControlMask && wattr.width == scr_width && wattr.height == scr_height) {
        XMoveWindow(dpy, win, wattr.x, wattr.height + 2 + MARGIN * 3);
    } else if (keycode == down_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y + scr_height / 8);
    } else if (keycode == left_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x - scr_width / 12, wattr.y);
    } else if (keycode == right_key && mods & ControlMask && wattr.width == scr_width && wattr.height == scr_height) {
        XMoveWindow(dpy, win, wattr.width + 2 + MARGIN * 3, wattr.y);
    } else if (keycode == right_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x + scr_width / 12, wattr.y);
    } else if ((keycode == down_key && (wattr.height == scr_height || wattr.height == scr_height + MARGIN * 2)) ||
           (keycode == up_key && (wattr.width == scr_width && wattr.height == scr_height)) || keycode == enter_key) {
        XMoveResizeWindow(dpy, win, last_attr.x, last_attr.y, last_attr.width, last_attr.height);
    } else if (keycode == up_key && wattr.y == top) {
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left, top, scr_width, scr_height);
    } else if (keycode == up_key) {
        XMoveWindow(dpy, win, wattr.x, top);
    } else if (keycode == down_key) {
        XMoveWindow(dpy, win, wattr.x, scr_height - wattr.height + top);
    } else if (keycode == left_key && (wattr.x == left || (wattr.width == scr_width && wattr.height == scr_height))) { 
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left, top, scr_width / 2 - MARGIN / 2, scr_height);
    } else if (keycode == left_key) {
        XMoveWindow(dpy, win, left, wattr.y);
    } else if (keycode == right_key &&
            ((wattr.x == scr_width - wattr.width + left) || (wattr.width == scr_width && wattr.height == scr_height))) {
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left + scr_width / 2 + MARGIN / 2, top, scr_width / 2 - MARGIN / 2, scr_height);
    } else if (keycode == right_key) {
        XMoveWindow(dpy, win, scr_width - wattr.width + left, wattr.y);
    }

    XRaiseWindow(dpy, win);
}

void HandleNewWindow(Window win) {
    XWindowAttributes wattr;
    XGetWindowAttributes(dpy, win, &wattr);

    if (win != None && !wattr.override_redirect && wattr.map_state == IsViewable && wattr.height < 100) {
        panelheight = wattr.height;
        XMoveWindow(dpy, win, 0, 0);
    } else if (win != None && !wattr.override_redirect && wattr.map_state == IsViewable) {
        XSelectInput(dpy, win, FocusChangeMask);
        XSetWindowBorderWidth(dpy, win, 1);
        XSetWindowBorder(dpy, win, 0);
        XRaiseWindow(dpy, win);
        XMoveWindow(dpy, Clients(1, True), wattr.x + MARGIN, panelheight + MARGIN);
        
        for (int i = 0; i < 512; i++) {
            if (!owins[i]) {
                owins[i] = win;
                break;
            }
        }
    }
}

void HandleMotion(XEvent ev) {
    int xdiff = ev.xbutton.x_root - click_ev.x_root;
    int ydiff = ev.xbutton.y_root - click_ev.y_root;

    if (click_ev.button == Button1) {
        XMoveWindow(dpy, ev.xmotion.window, last_attr.x + xdiff, last_attr.y + ydiff);
    } else if (click_ev.button == Button3) {
        XResizeWindow(dpy, ev.xmotion.window, abs(last_attr.width + xdiff) + 1, abs(last_attr.height + ydiff) + 1);
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
        for (unsigned int ikey = 1; ikey <= 9; ikey++) GrabKey(NUMCODE(ikey), Mod4Mask|mods[i]);
        for (unsigned int ikey = 0; ikey < sizeof(CMD_KEYS) / sizeof(CMD_KEYS[0]); ikey++) {
            GrabKey(GetKeycode(CMD_KEYS[ikey][0]), (CMD_KEYS[ikey][0][0] >= 'A' && CMD_KEYS[ikey][0][0] <= 'Z') ?
                    mods[i] : Mod4Mask|mods[i]);
        }
        GrabKey(enter_key = GetKeycode("Return"), Mod4Mask|mods[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod4Mask|mods[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod4Mask|mods[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod4Mask|mods[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod4Mask|mods[i]);
        GrabKey(f4_key    = GetKeycode("F4"),     Mod1Mask|mods[i]);
        GrabKey(w_key     = GetKeycode("w"),      ControlMask|ShiftMask|mods[i]);
        GrabKey(del_key   = GetKeycode("Delete"), ControlMask|Mod1Mask|mods[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),    Mod1Mask|mods[i]);
        GrabKey(tab_key,    Mod4Mask|mods[i]);
        GrabKey(tab_key,    ShiftMask|Mod4Mask|mods[i]);
        GrabKey(up_key,     ShiftMask|Mod4Mask|mods[i]);
        GrabKey(down_key,   ShiftMask|Mod4Mask|mods[i]);
        GrabKey(left_key,   ShiftMask|Mod4Mask|mods[i]);
        GrabKey(right_key,  ShiftMask|Mod4Mask|mods[i]);
        GrabKey(up_key,     ControlMask|Mod4Mask|mods[i]);
        GrabKey(down_key,   ControlMask|Mod4Mask|mods[i]);
        GrabKey(left_key,   ControlMask|Mod4Mask|mods[i]);
        GrabKey(right_key,  ControlMask|Mod4Mask|mods[i]);
        XGrabButton(dpy, AnyButton, Mod4Mask|mods[i], XDefaultRootWindow(dpy), True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);
    }
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(dpy, &ev);

    for (unsigned int i = 0; i < sizeof(CMD_KEYS) / sizeof(CMD_KEYS[0]); i++) {
        if (ev.type == KeyPress && ev.xkey.keycode == GetKeycode(CMD_KEYS[i][0])) system(CMD_KEYS[i][1]);
    }
    if (ev.type == KeyPress && ev.xkey.keycode >= NUMCODE(1) && ev.xkey.keycode <= NUMCODE(9)) {
        XRaiseWindow(dpy, owins[ev.xkey.keycode - 10] ? owins[ev.xkey.keycode - 10] : Clients(1, False));
    } else if (ev.type == KeyPress && ev.xkey.keycode == tab_key && ev.xkey.state & Mod1Mask) {
        XRaiseWindow(dpy, Clients(2, False));
    } else if (ev.type == KeyPress && ev.xkey.keycode == tab_key && ev.xkey.state & Mod4Mask) {
        int i = 0;
        for (; i < 512; i++) if (owins[i] == Clients(1, False)) break;
        if (ev.xkey.state & ShiftMask) XRaiseWindow(dpy, --i < 0 ? Clients(999999999, False) : owins[i]);
        else XRaiseWindow(dpy, owins[++i] ? owins[i] : owins[0]);
    } else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(dpy);
    } else if (ev.type == KeyPress && (ev.xkey.keycode == f4_key || ev.xkey.keycode == w_key)) {
        SendEvent(Clients(1, False), "WM_DELETE_WINDOW");
    } else if (ev.type == KeyPress) {
        HandleWindowPosition(Clients(1, False), ev.xkey.keycode, ev.xkey.state);
    } else if (ev.type == ButtonPress && ev.xbutton.window != XDefaultRootWindow(dpy)) {
        XRaiseWindow(dpy, ev.xbutton.window);
    } else if (ev.type == ButtonPress && ev.xbutton.window == XDefaultRootWindow(dpy)
            && ev.xbutton.subwindow != None && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
        XRaiseWindow(dpy, (click_ev = ev.xbutton).subwindow);
        XGrabPointer(dpy, ev.xbutton.subwindow, True, PointerMotionMask|ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        XGetWindowAttributes(dpy, ev.xbutton.subwindow, &last_attr);
    } else if (ev.type == ButtonRelease) {
        XUngrabPointer(dpy, CurrentTime);
    } else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    } else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    } else if (ev.type == UnmapNotify) {
        XSetInputFocus(dpy, Clients(1, True), RevertToPointerRoot, CurrentTime);
        for (int i = 0, j = i; i < 512; i++, j++) owins[i] = owins[i] == ev.xunmap.window ? owins[++j] : owins[j];
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

    for (unsigned int i = 0; i < sizeof(STARTUP_CMDS) / sizeof(STARTUP_CMDS[0]); i++) system(STARTUP_CMDS[i]);

    for(;;) InterceptEvents();
}
