/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <stdlib.h>
#include <X11/Xlib.h>

Display *display;
Window root_win;
XWindowAttributes root_attr;
XWindowAttributes win_attr;
XEvent ev;

KeyCode up_key;
KeyCode down_key;
KeyCode left_key;
KeyCode right_key;
KeyCode p_key;
KeyCode t_key;
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

int WeeGetKeycode(char *key) {
    return XKeysymToKeycode(display, XStringToKeysym(key));
}

void WeeSetupGrab() {
    // Intercept keys and mouse buttons. Mod2Mask=NumLock, Mod3Mask=ScrollLock, LockMask=CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };

    for (int i = 0; i < 8; i++) {
        WeeGrabKey(vol_up_key   = WeeGetKeycode("XF86AudioRaiseVolume"), modifiers[i]);
        WeeGrabKey(vol_down_key = WeeGetKeycode("XF86AudioLowerVolume"), modifiers[i]);
        WeeGrabKey(p_key     = WeeGetKeycode("p"), Mod1Mask|modifiers[i]);
        WeeGrabKey(t_key     = WeeGetKeycode("t"), ControlMask|Mod1Mask|modifiers[i]);
        WeeGrabKey(print_key = WeeGetKeycode("Print"), Mod1Mask|modifiers[i]);
        WeeGrabKey(tab_key   = WeeGetKeycode("Tab"),   Mod1Mask|modifiers[i]);
        WeeGrabKey(tab_key   = WeeGetKeycode("Tab"),   ShiftMask|Mod1Mask|modifiers[i]);
        WeeGrabKey(f4_key    = WeeGetKeycode("F4"),    Mod1Mask|modifiers[i]);
        WeeGrabKey(del_key   = WeeGetKeycode("Delete"),Mod4Mask|Mod1Mask|modifiers[i]);
        WeeGrabKey(up_key    = WeeGetKeycode("Up"),    Mod4Mask|modifiers[i]);
        WeeGrabKey(down_key  = WeeGetKeycode("Down"),  Mod4Mask|modifiers[i]);
        WeeGrabKey(left_key  = WeeGetKeycode("Left"),  Mod4Mask|modifiers[i]);
        WeeGrabKey(right_key = WeeGetKeycode("Right"), Mod4Mask|modifiers[i]);
        WeeGrabKey(up_key    = WeeGetKeycode("Up"),    ShiftMask|Mod4Mask|modifiers[i]);
        WeeGrabKey(down_key  = WeeGetKeycode("Down"),  ShiftMask|Mod4Mask|modifiers[i]);
        XGrabButton(display, 1, Mod1Mask|modifiers[i],
                root_win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    }
}

void WeeRunCmd(char *cmd) {
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
    Bool is_positioned = win_attr.x == root_attr.width - win_attr.width;

    if (win_attr.width == root_attr.width / 3 && is_positioned) {
        // Has 1/3 width at right. Resize to 1/2 at right
        XMoveWindow(display, win, root_attr.width / 2, win_attr.y);
        XResizeWindow(display, win, root_attr.width / 2, win_attr.height);
    }
    else if (win_attr.width == root_attr.width / 2 && is_positioned) {
        // Has 1/2 width at right. Resize to 2/3 at right
        XMoveWindow(display, win, root_attr.width / 3, win_attr.y);
        XResizeWindow(display, win, root_attr.width / 3 * 2, win_attr.height);
    }
    else {
        // Isn't at left. Move to left. Toggle width to 1/2 or 1/3 values
        int width = win_attr.width == root_attr.width / 2
            ? root_attr.width / 3
            : root_attr.width / 2;

        XMoveWindow(display, win, 1, win_attr.y);
        XResizeWindow(display, win, width, win_attr.height);
    }
}

void WeeResizeToRight(Window win) {
    Bool is_positioned = win_attr.x == 1;

    if (win_attr.width == root_attr.width / 3 && is_positioned) {
        // Has 1/3 width at left. Resize to 1/2 at left
        XResizeWindow(display, win, root_attr.width / 2, win_attr.height);
    }
    else if (win_attr.width == root_attr.width / 2 && is_positioned) {
        // Has 1/2 width at left. Resize to 2/3 at left
        XResizeWindow(display, win, root_attr.width / 3 * 2 + 1, win_attr.height);
    }
    else {
        // Isn't at right. Move to right. Toggle width to 1/2 or 1/3 values
        int width = win_attr.width == root_attr.width / 2
            ? root_attr.width / 3
            : root_attr.width / 2;

        XMoveWindow(display, win, root_attr.width - width, win_attr.y);
        XResizeWindow(display, win, width, win_attr.height);
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

void WeeInterceptEvents() {
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress) {
        XRaiseWindow(display, ev.xkey.subwindow);
        XSetInputFocus(display, ev.xkey.subwindow, RevertToPointerRoot, None); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == p_key) {
        WeeRunCmd("weema-cmd.sh launcher");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
        WeeRunCmd("weema-cmd.sh terminal");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
        WeeRunCmd("weema-cmd.sh volumeup");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
        WeeRunCmd("weema-cmd.sh volumedown");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
        WeeRunCmd("weema-cmd.sh printscreenarea");
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

    for(;;) {
        WeeInterceptEvents();
    }
}
