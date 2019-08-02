/* Weema is written by Henrique Gogó <henriquegogo@gmail.com>, 2019.
 * Inpired by Nick Welch's TinyWM.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include <X11/Xlib.h>

int main() {
    Display * dpy;
    Window root;
    XWindowAttributes rattr;
    XWindowAttributes wattr;
    XButtonEvent start;
    XEvent ev;

    KeyCode tab_key, up_key, down_key, left_key, right_key, f4_key, del_key;

    if (!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);
    XGetWindowAttributes(dpy, root, &rattr);
    XSelectInput(dpy, root, SubstructureNotifyMask);

    // Intercept keys and mouse buttons. Mod2Mask = NumLock, Mod3Mask = ScrollLock, LockMask = CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask, Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };
    for (int i = 0; i < 8; i++) {
        XGrabKey(dpy, tab_key   = XKeysymToKeycode(dpy, XStringToKeysym("Tab")),   Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, up_key    = XKeysymToKeycode(dpy, XStringToKeysym("Up")),    Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, down_key  = XKeysymToKeycode(dpy, XStringToKeysym("Down")),  Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, left_key  = XKeysymToKeycode(dpy, XStringToKeysym("Left")),  Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, right_key = XKeysymToKeycode(dpy, XStringToKeysym("Right")), Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, f4_key    = XKeysymToKeycode(dpy, XStringToKeysym("F4")),    Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, del_key   = XKeysymToKeycode(dpy, XStringToKeysym("Delete")),Mod4Mask|Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabButton(dpy, 1, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(dpy, 2, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(dpy, 3, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    }

    for(;;) {
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
            XCirculateSubwindowsUp(dpy, root);
        }
        else if (ev.xkey.keycode == del_key) {
            XCloseDisplay(dpy);
        }
        else if (ev.type == KeyPress && ev.xkey.subwindow != None) {
            XGetWindowAttributes(dpy, ev.xkey.subwindow, &wattr);

            if (ev.xkey.keycode == up_key) {
                XMoveWindow(dpy, ev.xkey.subwindow, 0, 0);
                XResizeWindow(dpy, ev.xkey.subwindow, rattr.width, rattr.height);
            }
            else if (ev.xkey.keycode == down_key) {
                XMoveWindow(dpy, ev.xkey.subwindow, rattr.width * 0.33 / 2, rattr.height * 0.33 / 2);
                XResizeWindow(dpy, ev.xkey.subwindow, rattr.width * 0.66, rattr.height * 0.66);
                XWarpPointer(dpy, None, ev.xkey.subwindow, None, None, None, None, rattr.width * 0.66 / 2, rattr.height * 0.66 / 2);
            }
            else if (ev.xkey.keycode == left_key) {
                Bool is_positioned = wattr.height == rattr.height && wattr.x == rattr.width - wattr.width && wattr.y == 0;
                if (wattr.width == rattr.width / 3 && is_positioned) {
                    XMoveWindow(dpy, ev.xkey.subwindow, rattr.width / 2, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, rattr.width / 2, rattr.height);
                }
                else if (wattr.width == rattr.width / 2 && is_positioned) {
                    XMoveWindow(dpy, ev.xkey.subwindow, rattr.width / 3, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, rattr.width / 3 * 2, rattr.height);
                }
                else {
                    int width = wattr.width == rattr.width / 2 ? rattr.width / 3 :  rattr.width / 2;
                    XMoveWindow(dpy, ev.xkey.subwindow, 0, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, width, rattr.height);
                    XWarpPointer(dpy, None, ev.xkey.subwindow, None, None, None, None, width / 2, rattr.height / 2);
                }
            }
            else if (ev.xkey.keycode == right_key) {
                Bool is_positioned = wattr.height == rattr.height && wattr.x == 0 && wattr.y == 0;
                if (wattr.width == rattr.width / 3 && is_positioned) {
                    XResizeWindow(dpy, ev.xkey.subwindow, rattr.width / 2, rattr.height);
                }
                else if (wattr.width == rattr.width / 2 && is_positioned) {
                    XResizeWindow(dpy, ev.xkey.subwindow, rattr.width / 3 * 2, rattr.height);
                }
                else {
                    int width = wattr.width == rattr.width / 2 ? rattr.width / 3 : rattr.width / 2;
                    XMoveWindow(dpy, ev.xkey.subwindow, rattr.width - width, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, width, rattr.height);
                    XWarpPointer(dpy, None, ev.xkey.subwindow, None, None, None, None, width / 2, rattr.height / 2);
                }
            }
            else if (ev.xkey.keycode == f4_key) {
                XEvent event;
                event.xclient.type = ClientMessage;
                event.xclient.window = ev.xkey.subwindow;
                event.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
                event.xclient.format = 32;
                event.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
                event.xclient.data.l[1] = CurrentTime;
                XSendEvent(dpy, ev.xkey.subwindow, False, NoEventMask, &event);
                XCirculateSubwindowsUp(dpy, root);
            }
        }
        else if (ev.type == KeyRelease && ev.xkey.subwindow != None) {
            XSetInputFocus(dpy, ev.xkey.subwindow, RevertToPointerRoot, CurrentTime);
        }
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            if (ev.xbutton.button == 3) XLowerWindow(dpy, ev.xbutton.subwindow);
            else {
                XRaiseWindow(dpy, ev.xbutton.subwindow);
                XSetInputFocus(dpy, ev.xbutton.subwindow, RevertToPointerRoot, CurrentTime);
            }
            XGrabPointer(dpy, ev.xbutton.subwindow, True, PointerMotionMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &wattr);
            start = ev.xbutton;
        }
        else if (ev.type == ButtonRelease) {
            XUngrabPointer(dpy, CurrentTime);
        }
        else if (ev.type == MotionNotify) {
            while (XCheckTypedEvent(dpy, MotionNotify, &ev));
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            if (start.button == 1) XMoveWindow(dpy, ev.xmotion.window, wattr.x + xdiff, wattr.y + ydiff);
            else if (start.button == 2) XResizeWindow(dpy, ev.xmotion.window, wattr.width + xdiff, wattr.height + ydiff);
        }
        else if (ev.type == CirculateNotify) {
            XGetWindowAttributes(dpy, ev.xcirculate.window, &wattr);
            XWarpPointer(dpy, None, ev.xcirculate.window, None, None, None, None, wattr.width / 2, wattr.height / 2);
        }
    }
}
