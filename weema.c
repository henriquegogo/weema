/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <stdlib.h>
#include <X11/Xlib.h>

Display *display = NULL;
Window root_win;
XWindowAttributes root_attr;
XWindowAttributes win_attr;
XButtonEvent click_start;
XEvent ev;

KeyCode up_key;
KeyCode down_key;
KeyCode left_key;
KeyCode right_key;
KeyCode r_key;
KeyCode t_key;
KeyCode l_key;
KeyCode vol_up_key;
KeyCode vol_down_key;
KeyCode f4_key;
KeyCode del_key;
KeyCode tab_key;
KeyCode print_key;

void WeeInitRootWindow() {
    root_win = XDefaultRootWindow(display);
    XGetWindowAttributes(display, root_win, &root_attr);
    XSelectInput(display, root_win, SubstructureNotifyMask);
}

void WeeGrabKey(int keycode, unsigned int modifiers) {
    XGrabKey(display, keycode, modifiers, root_win, True, GrabModeAsync, GrabModeAsync);
}

void WeeGrabButton(int buttoncode, unsigned int modifiers) {
    XGrabButton(display, buttoncode, modifiers, root_win, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

int WeeGetKeycode(const char *key) {
    return XKeysymToKeycode(display, XStringToKeysym(key));
}

void WeeSetupGrab() {
    // Intercept keys and mouse buttons. Mod2Mask=NumLock, Mod3Mask=ScrollLock, LockMask=CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };

    for (int i = 0; i < 8; i++) {
        WeeGrabKey(vol_up_key   = WeeGetKeycode("XF86AudioRaiseVolume"), modifiers[i]);
        WeeGrabKey(vol_down_key = WeeGetKeycode("XF86AudioLowerVolume"), modifiers[i]);
        WeeGrabKey(r_key     = WeeGetKeycode("r"),     Mod4Mask|modifiers[i]);
        WeeGrabKey(t_key     = WeeGetKeycode("t"),     Mod4Mask|modifiers[i]);
        WeeGrabKey(l_key     = WeeGetKeycode("l"),     Mod4Mask|modifiers[i]);
        WeeGrabKey(print_key = WeeGetKeycode("Print"), modifiers[i]);
        WeeGrabKey(tab_key   = WeeGetKeycode("Tab"),   Mod1Mask|modifiers[i]);
        WeeGrabKey(tab_key   = WeeGetKeycode("Tab"),   ShiftMask|Mod1Mask|modifiers[i]);
        WeeGrabKey(f4_key    = WeeGetKeycode("F4"),    Mod1Mask|modifiers[i]);
        WeeGrabKey(del_key   = WeeGetKeycode("Delete"),ControlMask|Mod1Mask|modifiers[i]);
        WeeGrabKey(up_key    = WeeGetKeycode("Up"),    Mod4Mask|modifiers[i]);
        WeeGrabKey(down_key  = WeeGetKeycode("Down"),  Mod4Mask|modifiers[i]);
        WeeGrabKey(left_key  = WeeGetKeycode("Left"),  Mod4Mask|modifiers[i]);
        WeeGrabKey(right_key = WeeGetKeycode("Right"), Mod4Mask|modifiers[i]);
        WeeGrabKey(up_key    = WeeGetKeycode("Up"),    ShiftMask|Mod4Mask|modifiers[i]);
        WeeGrabKey(down_key  = WeeGetKeycode("Down"),  ShiftMask|Mod4Mask|modifiers[i]);
        WeeGrabKey(left_key  = WeeGetKeycode("Left"),  ShiftMask|Mod4Mask|modifiers[i]);
        WeeGrabKey(right_key = WeeGetKeycode("Right"), ShiftMask|Mod4Mask|modifiers[i]);
        WeeGrabButton(1, Mod1Mask|modifiers[i]);
    }
}

void WeeRunCmd(const char *cmd) {
    (void)(system(cmd)+1);
}

void WeeCloseWindow(Window win) {
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
    event.xclient.format = 32;
    event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
    event.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, win, False, NoEventMask, &event);
}

void WeeMoveCursor(Window win) {
    XGetWindowAttributes(display, win, &win_attr);
    XWarpPointer(display, None, win, None, None, None, None,
            win_attr.width / 2, win_attr.height / 2);
}

void WeeResizeFullScreen(Window win) {
    XMoveWindow(display, win, 0, 0);
    XResizeWindow(display, win, root_attr.width, root_attr.height);
}

void WeeResizeFullHeight(Window win) {
    XMoveWindow(display, win, win_attr.x, 0);
    XResizeWindow(display, win, win_attr.width, root_attr.height);
}

void WeeResizeFloatCentralized(Window win) {
    XMoveWindow(display, win, root_attr.width * 0.33 / 2, root_attr.height * 0.33 / 2);
    XResizeWindow(display, win, root_attr.width * 0.66, root_attr.height * 0.66);
}

void WeeResizeToTop(Window win) {
    XMoveWindow(display, win, win_attr.x, 0);
    XResizeWindow(display, win, win_attr.width, root_attr.height / 2 + 1);
}

void WeeResizeToBottom(Window win) {
    XMoveWindow(display, win, win_attr.x, root_attr.height / 2);
    XResizeWindow(display, win, win_attr.width, root_attr.height / 2 + 1);
}

void WeeResizeToLeft(Window win) {
    Bool at_left = win_attr.x == 1;
    Bool at_right = win_attr.x == root_attr.width - win_attr.width;
    int half = root_attr.width / 2;
    int third = root_attr.width / 3;

    if (win_attr.width == third && at_right) {
        XMoveWindow(display, win, half, win_attr.y);
        XResizeWindow(display, win, half, win_attr.height);
    }
    else if (win_attr.width == half && at_right) {
        XMoveWindow(display, win, third, win_attr.y);
        XResizeWindow(display, win, third * 2, win_attr.height);
    }
    else if ((win_attr.width == half || win_attr.width == third) && at_left) {
        XMoveWindow(display, win, 1, win_attr.y);
        XResizeWindow(display, win, third, win_attr.height);
    }
    else {
        XMoveWindow(display, win, 1, win_attr.y);
        XResizeWindow(display, win, half, win_attr.height);
    }
}

void WeeResizeToRight(Window win) {
    Bool at_left = win_attr.x == 1;
    Bool at_right = win_attr.x == root_attr.width - win_attr.width;
    int half = root_attr.width / 2;
    int third = root_attr.width / 3;

    if (win_attr.width == third && at_left) {
        XResizeWindow(display, win, half, win_attr.height);
    }
    else if (win_attr.width == half && at_left) {
        XResizeWindow(display, win, third * 2 + 1, win_attr.height);
    }
    else if ((win_attr.width == half || win_attr.width == third) && at_right) {
        XMoveWindow(display, win, root_attr.width - third, win_attr.y);
        XResizeWindow(display, win, third, win_attr.height);
    }
    else {
        XMoveWindow(display, win, root_attr.width - half, win_attr.y);
        XResizeWindow(display, win, half, win_attr.height);
    }
}

void WeeHandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XGetWindowAttributes(display, win, &win_attr);

    if (keycode == up_key && modifiers & ShiftMask) {
        WeeResizeFullScreen(win);
        WeeMoveCursor(win);
    }
    else if (keycode == down_key && modifiers & ShiftMask) {
        WeeResizeFloatCentralized(win);
        WeeMoveCursor(win);
    }
    else if (keycode == left_key && modifiers & ShiftMask) {
        WeeResizeToLeft(win);
        WeeMoveCursor(win);
        WeeResizeFullHeight(win);
    }
    else if (keycode == right_key && modifiers & ShiftMask) {
        WeeResizeToRight(win);
        WeeMoveCursor(win);
        WeeResizeFullHeight(win);
    }
    else if (keycode == up_key) {
        WeeResizeToTop(win);
        WeeMoveCursor(win);
    }
    else if (keycode == down_key) {
        WeeResizeToBottom(win);
        WeeMoveCursor(win);
    }
    else if (keycode == left_key) {
        WeeResizeToLeft(win);
        WeeMoveCursor(win);
    }
    else if (keycode == right_key) {
        WeeResizeToRight(win);
        WeeMoveCursor(win);
    }
}

void WeeRaiseAndFocus(Window win) {
    XRaiseWindow(display, win);
    XSetInputFocus(display, win, RevertToPointerRoot, None); 
}

void WeeHandleClick(XButtonEvent button_event) {
    XGrabPointer(display, button_event.subwindow, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(display, button_event.subwindow, &win_attr);
    click_start = button_event;
}

void WeeHandleMotion() {
    while (XCheckTypedEvent(display, MotionNotify, &ev));
    int xdiff = ev.xbutton.x_root - click_start.x_root;
    int ydiff = ev.xbutton.y_root - click_start.y_root;
    XMoveWindow(display, ev.xmotion.window, win_attr.x + xdiff, win_attr.y + ydiff);
}

void WeeInterceptEvents() {
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress) {
        WeeRaiseAndFocus(ev.xbutton.subwindow);
        WeeHandleClick(ev.xbutton);
    }
    else if (ev.type == ButtonRelease) {
        XUngrabPointer(display, CurrentTime);
    }
    else if (ev.type == MotionNotify) {
        WeeHandleMotion();
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == r_key) {
        WeeRunCmd("weema-cmd launcher &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
        WeeRunCmd("x-terminal-emulator &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == l_key) {
        WeeRunCmd("weema-cmd lock &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
        WeeRunCmd("weema-cmd volumeup &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
        WeeRunCmd("weema-cmd volumedown &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
        WeeRunCmd("weema-cmd printscreen &");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key && ev.xkey.state & ShiftMask) {
        // Move two cicles down and one up to focus last raised window
        XCirculateSubwindowsDown(display, root_win);
        XCirculateSubwindowsDown(display, root_win);
        XCirculateSubwindowsUp(display, root_win);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        XCirculateSubwindowsUp(display, root_win);
    }
    else if (ev.type == CirculateNotify) {
        XGetWindowAttributes(display, ev.xcirculate.window, &win_attr);
        WeeMoveCursor(ev.xcirculate.window);
        XSetInputFocus(display, ev.xcirculate.window, RevertToParent, None);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(display);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == f4_key) {
        WeeCloseWindow(ev.xkey.subwindow);
    }
    else if (ev.type == KeyPress && ev.xkey.subwindow != None) {
        WeeHandleWindowPosition(ev.xkey.subwindow, ev.xkey.keycode, ev.xkey.state);
    }
}

int main() {
    if (!(display = XOpenDisplay(0x0))) return 1;

    WeeInitRootWindow();
    WeeSetupGrab();
    WeeRunCmd("weema-cmd init &");

    for(;;) {
        WeeInterceptEvents();
    }
}
