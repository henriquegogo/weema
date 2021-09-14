/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2021.
 * MIT License */

#include <X11/Xlib.h>
#include <stdlib.h>
#include "config.h"

Display *display = NULL;
XButtonEvent clicked_event;
XWindowAttributes clicked_win_attr;
int current_screen_width, current_screen_height;
int top = 0, left = 0;

KeyCode up_key, down_key, left_key, right_key, r_key, t_key, l_key, b_key, w_key,
        vol_up_key, vol_down_key, f4_key, del_key, tab_key, print_key;

Window GetWindow(unsigned int win_i) {
    unsigned int nwins, count = 1;
    Window win, *wins;
    XQueryTree(display, XDefaultRootWindow(display), &win, &win, &wins, &nwins);
    top = 0;

    for (int i = nwins - 1; i > 0; i--) {
        XWindowAttributes win_attr;
        XGetWindowAttributes(display, wins[i], &win_attr);

        if (win_attr.height <= 64 && win_attr.y == 0 && win_attr.map_state == IsViewable) {
            top = win_attr.height;
        } else if (wins[i] != None && !win_attr.override_redirect && win_attr.map_state == IsViewable && win_i >= count++) {
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
    ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, win, False, NoEventMask, &ev);
}

void SetupScreen(XWindowAttributes win_attr) {
    current_screen_width = win_attr.screen->width;
    current_screen_height = win_attr.screen->height;
    left = 0;

    if (win_attr.x >= win_attr.screen->width) {
        XWindowAttributes root_attr;
        XGetWindowAttributes(display, XDefaultRootWindow(display), &root_attr);
        left = current_screen_width;
        current_screen_width = root_attr.width - win_attr.screen->width;
        current_screen_height = root_attr.height;
    }
}

void ResizeUp(Window win, XWindowAttributes win_attr) {
    if (win_attr.height <= 2 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 4 - top);
    } else if (win_attr.height <= 3 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, 2 * current_screen_height / 4 - top);
    } else if (win_attr.height <= current_screen_height - top) {
        XResizeWindow(display, win, win_attr.width, 3 * current_screen_height / 4 - top);
    }
}

void ResizeDown(Window win, XWindowAttributes win_attr) {
    if (win_attr.height < 2 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, 2 * current_screen_height / 4 - top);
    } else if (win_attr.height < 3 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, 3 * current_screen_height / 4 - top);
    } else {
        XMoveResizeWindow(display, win, win_attr.x, top, win_attr.width, current_screen_height - top);
    }
}

void ResizeLeft(Window win, XWindowAttributes win_attr) {
    if (win_attr.width <= 2 * current_screen_width / 4) {
        XResizeWindow(display, win, current_screen_width / 4, win_attr.height);
    } else if (win_attr.width <= 3 * current_screen_width / 4) {
        XResizeWindow(display, win, 2 * current_screen_width / 4, win_attr.height);
    } else if (win_attr.width <= current_screen_width) {
        XResizeWindow(display, win, 3 * current_screen_width / 4, win_attr.height);
    }
}

void ResizeRight(Window win, XWindowAttributes win_attr) {
    if (win_attr.width < 2 * current_screen_width / 4) {
        XResizeWindow(display, win, 2 * current_screen_width / 4, win_attr.height);
    } else if (win_attr.width < 3 * current_screen_width / 4) {
        XResizeWindow(display, win, 3 * current_screen_width / 4, win_attr.height);
    } else {
        XMoveResizeWindow(display, win, left, win_attr.y, current_screen_width, win_attr.height);
    }
}

void PositionUp(Window win, XWindowAttributes win_attr) {
    if (win_attr.y == top) {
        XMoveResizeWindow(display, win, left, top, current_screen_width, current_screen_height - top);
    } else {
        XMoveWindow(display, win, win_attr.x, top);
    }
}

void PositionDown(Window win, XWindowAttributes win_attr) {
    if (win_attr.width == current_screen_width && win_attr.height == current_screen_height - top) {
        XMoveResizeWindow(display, win, current_screen_width / 3 / 2 + left, current_screen_height / 3 / 2,
                current_screen_width / 3 * 2, current_screen_height / 3 * 2);
    } else {
        XMoveWindow(display, win, win_attr.x, current_screen_height - win_attr.height);
    }
}

void PositionLeft(Window win, XWindowAttributes win_attr) {
    if (win_attr.x == left) {
        XMoveResizeWindow(display, win, left, top, win_attr.width, current_screen_height - top);
    } else {
        XMoveWindow(display, win, left, win_attr.y);
    }
}

void PositionRight(Window win, XWindowAttributes win_attr) {
    int right_pos = current_screen_width - win_attr.width + left;

    if (win_attr.x == right_pos) {
        XMoveResizeWindow(display, win, right_pos, top, win_attr.width, current_screen_height - top);
    } else {
        XMoveWindow(display, win, current_screen_width - win_attr.width + left, win_attr.y);
    }
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);
    SetupScreen(win_attr);

    if (keycode == up_key && modifiers & ShiftMask) {
        ResizeUp(win, win_attr);
    } else if (keycode == down_key && modifiers & ShiftMask) {
        ResizeDown(win, win_attr);
    } else if (keycode == left_key && modifiers & ShiftMask) {
        ResizeLeft(win, win_attr);
    } else if (keycode == right_key && modifiers & ShiftMask) {
        ResizeRight(win, win_attr);
    } else if (keycode == up_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x, win_attr.y - current_screen_height / 4);
    } else if (keycode == down_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x, win_attr.y + current_screen_height / 4);
    } else if (keycode == left_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x - current_screen_width / 4, win_attr.y);
    } else if (keycode == right_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x + current_screen_width / 4, win_attr.y);
    } else if (keycode == up_key) {
        PositionUp(win, win_attr);
    } else if (keycode == down_key) {
        PositionDown(win, win_attr);
    } else if (keycode == left_key) {
        PositionLeft(win, win_attr);
    } else if (keycode == right_key) {
        PositionRight(win, win_attr);
    }

    XRaiseWindow(display, win);
}

void HandleNewWindow(Window win) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);

    if (win != None && !win_attr.override_redirect && win_attr.map_state == IsViewable) {
        XSetWindowBorderWidth(display, win, 1);
        XSetWindowBorder(display, win, 0);
        XRaiseWindow(display, win);
        XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
        XSelectInput(display, win, FocusChangeMask);
        XMoveWindow(display, GetWindow(1), win_attr.x, top);
    }
}

void HandleClick(XButtonEvent button_event) {
    XGrabPointer(display, button_event.subwindow, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(display, button_event.subwindow, &clicked_win_attr);
    clicked_event = button_event;
}

void HandleMotion(XEvent ev) {
    int xdiff = ev.xbutton.x_root - clicked_event.x_root;
    int ydiff = ev.xbutton.y_root - clicked_event.y_root;

    if (clicked_event.button == Button1) {
        XMoveWindow(display, ev.xmotion.window, clicked_win_attr.x + xdiff, clicked_win_attr.y + ydiff);
    } else if (clicked_event.button == Button3) {
        XResizeWindow(display, ev.xmotion.window,
                abs(clicked_win_attr.width + xdiff) + 1, abs(clicked_win_attr.height + ydiff) + 1);
    }
}

void GrabKey(int keycode, unsigned int modifiers) {
    XGrabKey(display, keycode, modifiers, XDefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
}

int GetKeycode(const char *key) {
    return XKeysymToKeycode(display, XStringToKeysym(key));
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
        XGrabButton(display, AnyButton, Mod1Mask|modifiers[i], XDefaultRootWindow(display), True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);
    }
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress && ev.xbutton.window != XDefaultRootWindow(display)) {
        XRaiseWindow(display, ev.xbutton.window);
    } else if (ev.type == ButtonPress && ev.xbutton.window == XDefaultRootWindow(display)
            && ev.xbutton.subwindow != None && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
        XRaiseWindow(display, ev.xbutton.subwindow);
        HandleClick(ev.xbutton);
    } else if (ev.type == ButtonRelease) {
        XUngrabPointer(display, CurrentTime);
    } else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    } else if (ev.type == KeyPress && ev.xkey.keycode == r_key) {
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
    } else if (ev.type == KeyPress && ev.xkey.keycode == tab_key && ev.xkey.state & ShiftMask) {
        XRaiseWindow(display, GetWindow(999999999));
    } else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        XRaiseWindow(display, GetWindow(2));
    } else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(display);
        exit(0);
    } else if (ev.type == KeyPress && (ev.xkey.keycode == f4_key || ev.xkey.keycode == w_key)) {
        CloseWindow(GetWindow(1));
    } else if (ev.type == KeyPress) {
        HandleWindowPosition(GetWindow(1), ev.xkey.keycode, ev.xkey.state);
    } else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    } else if (ev.type == UnmapNotify) {
        XSetInputFocus(display, GetWindow(1), RevertToPointerRoot, CurrentTime);
    } else if (ev.type == FocusIn) {
        XUngrabButton(display, AnyButton, AnyModifier, ev.xfocus.window);
        XAllowEvents(display, ReplayPointer, CurrentTime);
    } else if (ev.type == FocusOut) {
        XGrabButton(display, AnyButton, AnyModifier, ev.xfocus.window, True, ButtonPressMask,
                GrabModeSync, GrabModeSync, None, None);
    } else if (ev.type == ConfigureNotify) {
        XSetInputFocus(display, ev.xconfigure.window, RevertToPointerRoot, CurrentTime); 
    }
}

int ErrorHandler() {
    return 0;
}

int main() {
    if (!(display = XOpenDisplay(NULL))) return 1;

    XSetErrorHandler(ErrorHandler);
    XSelectInput(display, XDefaultRootWindow(display), SubstructureNotifyMask);
    SetupGrab(); 
    system("xsetroot -cursor_name arrow -solid \"#030609\"");
    system(CMD_INIT);

    for(;;) InterceptEvents();
}
