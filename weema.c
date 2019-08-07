/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2019.
 * MIT License */

#include <X11/Xlib.h>

Display * display;
Window root_win;
XWindowAttributes root_attr;
XWindowAttributes win_attr;
XButtonEvent click_start;
XEvent ev;

KeyCode tab_key, up_key, down_key, left_key, right_key, f4_key, del_key;

void setup() {
    root_win = DefaultRootWindow(display);
    XGetWindowAttributes(display, root_win, &root_attr);
    XSelectInput(display, root_win, SubstructureNotifyMask|EnterWindowMask);

    // Intercept keys and mouse buttons. Mod2Mask = NumLock, Mod3Mask = ScrollLock, LockMask = CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask, Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };
    for (int i = 0; i < 8; i++) {
        XGrabKey(display, tab_key   = XKeysymToKeycode(display, XStringToKeysym("Tab")),   Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, up_key    = XKeysymToKeycode(display, XStringToKeysym("Up")),    Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, down_key  = XKeysymToKeycode(display, XStringToKeysym("Down")),  Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, left_key  = XKeysymToKeycode(display, XStringToKeysym("Left")),  Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, right_key = XKeysymToKeycode(display, XStringToKeysym("Right")), Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, f4_key    = XKeysymToKeycode(display, XStringToKeysym("F4")),    Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, del_key   = XKeysymToKeycode(display, XStringToKeysym("Delete")),Mod4Mask|Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabButton(display, 1, Mod1Mask|modifiers[i], root_win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(display, 2, Mod1Mask|modifiers[i], root_win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(display, 3, Mod1Mask|modifiers[i], root_win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    }
}

void close_window(Window win) {
    XCirculateSubwindowsDown(display, root_win);
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
    event.xclient.format = 32;
    event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
    event.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, win, False, NoEventMask, &event);
}

void draw_border(Window win) {
    if (win != None) {
        XSetWindowBorderWidth(display, win, 1);
        XSetWindowBorder(display, win, 0);
    }
}

void centralize_mouse(Window win) {
    XGetWindowAttributes(display, win, &win_attr);
    XWarpPointer(display, None, win, None, None, None, None, win_attr.width / 2, win_attr.height / 2);
}

void handle_click(XButtonEvent click_event) {
    if (click_event.subwindow != None) {
        if (click_event.button == 3) XLowerWindow(display, click_event.subwindow);
        else XRaiseWindow(display, click_event.subwindow);
        XGrabPointer(display, click_event.subwindow, True, PointerMotionMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        XGetWindowAttributes(display, click_event.subwindow, &win_attr);
        click_start = click_event;
    }
}

void handle_motion() {
    while (XCheckTypedEvent(display, MotionNotify, &ev));
    int xdiff = ev.xbutton.x_root - click_start.x_root;
    int ydiff = ev.xbutton.y_root - click_start.y_root;
    if (click_start.button == 1) XMoveWindow(display, ev.xmotion.window, win_attr.x + xdiff, win_attr.y + ydiff);
    else if (click_start.button == 2) XResizeWindow(display, ev.xmotion.window, win_attr.width + xdiff, win_attr.height + ydiff);
}

void handle_arrow_keys(XKeyEvent key_event) {
    if (key_event.subwindow != None) {
        XGetWindowAttributes(display, key_event.subwindow, &win_attr);

        if (key_event.keycode == up_key) {
            XMoveWindow(display, key_event.subwindow, 0, 0);
            XResizeWindow(display, key_event.subwindow, root_attr.width, root_attr.height);
        }
        else if (key_event.keycode == down_key) {
            XMoveWindow(display, key_event.subwindow, root_attr.width * 0.33 / 2, root_attr.height * 0.33 / 2);
            XResizeWindow(display, key_event.subwindow, root_attr.width * 0.66, root_attr.height * 0.66);
            XWarpPointer(display, None, key_event.subwindow, None, None, None, None, root_attr.width * 0.66 / 2, root_attr.height * 0.66 / 2);
        }
        else if (key_event.keycode == left_key) {
            Bool is_positioned = win_attr.height == root_attr.height && win_attr.x == root_attr.width - win_attr.width && win_attr.y == 0;
            if (win_attr.width == root_attr.width / 3 && is_positioned) {
                XMoveWindow(display, key_event.subwindow, root_attr.width / 2, 0);
                XResizeWindow(display, key_event.subwindow, root_attr.width / 2, root_attr.height);
            }
            else if (win_attr.width == root_attr.width / 2 && is_positioned) {
                XMoveWindow(display, key_event.subwindow, root_attr.width / 3, 0);
                XResizeWindow(display, key_event.subwindow, root_attr.width / 3 * 2, root_attr.height);
            }
            else {
                int width = win_attr.width == root_attr.width / 2 ? root_attr.width / 3 :  root_attr.width / 2;
                XMoveWindow(display, key_event.subwindow, 0, 0);
                XResizeWindow(display, key_event.subwindow, width, root_attr.height);
                XWarpPointer(display, None, key_event.subwindow, None, None, None, None, width / 2, root_attr.height / 2);
            }
        }
        else if (key_event.keycode == right_key) {
            Bool is_positioned = win_attr.height == root_attr.height && win_attr.x == 0 && win_attr.y == 0;
            if (win_attr.width == root_attr.width / 3 && is_positioned) {
                XResizeWindow(display, key_event.subwindow, root_attr.width / 2, root_attr.height);
            }
            else if (win_attr.width == root_attr.width / 2 && is_positioned) {
                XResizeWindow(display, key_event.subwindow, root_attr.width / 3 * 2, root_attr.height);
            }
            else {
                int width = win_attr.width == root_attr.width / 2 ? root_attr.width / 3 : root_attr.width / 2;
                XMoveWindow(display, key_event.subwindow, root_attr.width - width, 0);
                XResizeWindow(display, key_event.subwindow, width, root_attr.height);
                XWarpPointer(display, None, key_event.subwindow, None, None, None, None, width / 2, root_attr.height / 2);
            }
        }
    }
}

int main() {
    if (!(display = XOpenDisplay(0x0))) return 1;

    setup();

    for(;;) {
        XNextEvent(display, &ev);

        if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
            XCirculateSubwindowsUp(display, root_win);
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
            XCloseDisplay(display);
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == f4_key && ev.xkey.subwindow != None) {
            close_window(ev.xkey.subwindow);
        }
        else if (ev.type == KeyPress) {
            handle_arrow_keys(ev.xkey);
        }
        else if (ev.type == ButtonPress) {
            handle_click(ev.xbutton);
        }
        else if (ev.type == ButtonRelease) {
            XUngrabPointer(display, CurrentTime);
        }
        else if (ev.type == MotionNotify) {
            handle_motion();
        }
        else if (ev.type == CirculateNotify) {
            centralize_mouse(ev.xcirculate.window);
        }
    }
}
