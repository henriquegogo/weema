/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <X11/X.h>
#include <stdlib.h>
#include <X11/Xlib.h>

Display * display;
Window root_win;
XWindowAttributes root_attr;
XWindowAttributes win_attr;
XButtonEvent click_start;
XEvent ev;

KeyCode tab_key, up_key, down_key, left_key, right_key, f4_key, del_key, p_key, t_key, vol_up_key, vol_down_key, print_key;

void setup() {
    root_win = XDefaultRootWindow(display);
    XGetWindowAttributes(display, root_win, &root_attr);
    XSelectInput(display, root_win, SubstructureNotifyMask);

    // Intercept keys and mouse buttons. Mod2Mask = NumLock, Mod3Mask = ScrollLock, LockMask = CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask, Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };
    for (int i = 0; i < 8; i++) {
        XGrabButton(display, 1, Mod1Mask|modifiers[i], root_win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabKey(display, p_key     = XKeysymToKeycode(display, XStringToKeysym("p")),     Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, t_key     = XKeysymToKeycode(display, XStringToKeysym("t")),     ControlMask|Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, vol_up_key   = XKeysymToKeycode(display, XStringToKeysym("XF86AudioRaiseVolume")), modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, vol_down_key = XKeysymToKeycode(display, XStringToKeysym("XF86AudioLowerVolume")), modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, print_key = XKeysymToKeycode(display, XStringToKeysym("Print")), Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, tab_key   = XKeysymToKeycode(display, XStringToKeysym("Tab")),   Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, f4_key    = XKeysymToKeycode(display, XStringToKeysym("F4")),    Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, del_key   = XKeysymToKeycode(display, XStringToKeysym("Delete")),Mod4Mask|Mod1Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, up_key    = XKeysymToKeycode(display, XStringToKeysym("Up")),    Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, down_key  = XKeysymToKeycode(display, XStringToKeysym("Down")),  Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, left_key  = XKeysymToKeycode(display, XStringToKeysym("Left")),  Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, right_key = XKeysymToKeycode(display, XStringToKeysym("Right")), Mod4Mask|modifiers[i], root_win, True, GrabModeAsync, GrabModeAsync);
    }
}

int main() {
    if (!(display = XOpenDisplay(0x0))) return 1;

    setup();

    for(;;) {
        XNextEvent(display, &ev);

        if (ev.type == ButtonPress) {
            XRaiseWindow(display, ev.xkey.subwindow);
            XSetInputFocus(display, ev.xkey.subwindow, RevertToPointerRoot, None); 
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == p_key) {
            system("weema-cmd.sh launcher");
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
            system("weema-cmd.sh terminal");
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
            system("weema-cmd.sh volumeup");
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
            system("weema-cmd.sh volumedown");
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
            system("weema-cmd.sh printscreenarea");
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
            XCirculateSubwindowsUp(display, root_win);
        }
        else if (ev.type == CirculateNotify) {
            XGetWindowAttributes(display, ev.xcirculate.window, &win_attr);
            XWarpPointer(display, None, ev.xcirculate.window, None, None, None, None, win_attr.width / 2, win_attr.height / 2);
            XSetInputFocus(display, ev.xcirculate.window, RevertToParent, None);
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
            XCloseDisplay(display);
        }
        else if (ev.type == KeyPress && ev.xkey.keycode == f4_key) {
            XEvent event;
            event.xclient.type = ClientMessage;
            event.xclient.window = ev.xkey.subwindow;
            event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
            event.xclient.format = 32;
            event.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
            event.xclient.data.l[1] = CurrentTime;
            XSendEvent(display, ev.xkey.subwindow, False, NoEventMask, &event);
        }
        else if (ev.type == KeyPress) {
            if (ev.xkey.subwindow != None) {
                XGetWindowAttributes(display, ev.xkey.subwindow, &win_attr);

                if (ev.xkey.keycode == up_key) {
                    // Full screen
                    XMoveWindow(display, ev.xkey.subwindow, 0, 0);
                    XResizeWindow(display, ev.xkey.subwindow, root_attr.width, root_attr.height);
                }
                else if (ev.xkey.keycode == down_key) {
                    // Float window centralized
                    XMoveWindow(display, ev.xkey.subwindow, root_attr.width * 0.33 / 2, root_attr.height * 0.33 / 2);
                    XResizeWindow(display, ev.xkey.subwindow, root_attr.width * 0.66, root_attr.height * 0.66);
                    XWarpPointer(display, None, ev.xkey.subwindow, None, None, None, None, root_attr.width * 0.66 / 2, root_attr.height * 0.66 / 2);
                }
                else if (ev.xkey.keycode == left_key) {
                    Bool is_positioned = win_attr.height == root_attr.height && win_attr.x == root_attr.width - win_attr.width && win_attr.y == 0;
                    if (win_attr.width == root_attr.width / 3 && is_positioned) {
                        // Has 1/3 width at right. Resize to 1/2 at right
                        XMoveWindow(display, ev.xkey.subwindow, root_attr.width / 2, 0);
                        XResizeWindow(display, ev.xkey.subwindow, root_attr.width / 2, root_attr.height);
                    }
                    else if (win_attr.width == root_attr.width / 2 && is_positioned) {
                        // Has 1/2 width at right. Resize to 2/3 at right
                        XMoveWindow(display, ev.xkey.subwindow, root_attr.width / 3, 0);
                        XResizeWindow(display, ev.xkey.subwindow, root_attr.width / 3 * 2, root_attr.height);
                    }
                    else {
                        // Isn't at left. Move to left. Toggle width to 1/2 or 1/3 values
                        int width = win_attr.width == root_attr.width / 2 ? root_attr.width / 3 : root_attr.width / 2;
                        XMoveWindow(display, ev.xkey.subwindow, 1, 0);
                        XResizeWindow(display, ev.xkey.subwindow, width, root_attr.height);
                        XWarpPointer(display, None, ev.xkey.subwindow, None, None, None, None, width / 2, root_attr.height / 2);
                    }
                }
                else if (ev.xkey.keycode == right_key) {
                    Bool is_positioned = win_attr.height == root_attr.height && win_attr.x == 1 && win_attr.y == 0;
                    if (win_attr.width == root_attr.width / 3 && is_positioned) {
                        // Has 1/3 width at left. Resize to 1/2 at left
                        XResizeWindow(display, ev.xkey.subwindow, root_attr.width / 2, root_attr.height);
                    }
                    else if (win_attr.width == root_attr.width / 2 && is_positioned) {
                        // Has 1/2 width at left. Resize to 2/3 at left
                        XResizeWindow(display, ev.xkey.subwindow, root_attr.width / 3 * 2 + 1, root_attr.height);
                    }
                    else {
                        // Isn't at right. Move to right. Toggle width to 1/2 or 1/3 values
                        int width = win_attr.width == root_attr.width / 2 ? root_attr.width / 3 : root_attr.width / 2;
                        XMoveWindow(display, ev.xkey.subwindow, root_attr.width - width, 0);
                        XResizeWindow(display, ev.xkey.subwindow, width, root_attr.height);
                        XWarpPointer(display, None, ev.xkey.subwindow, None, None, None, None, width / 2, root_attr.height / 2);
                    }
                }
            }
        }
    }
}
