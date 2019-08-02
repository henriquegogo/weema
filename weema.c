/* Weema is written by Henrique Gogó <henriquegogo@gmail.com>, 2019.
 * Inpired by Nick Welch's TinyWM.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include <X11/Xlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main() {
    Display * dpy;
    Window root;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    KeyCode tab_key, up_key, left_key, right_key, f4_key, del_key;

    if (!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, SubstructureNotifyMask);

    // Intercept keys and mouse buttons. Mod2Mask = NumLock, Mod3Mask = ScrollLock, LockMask = CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask, Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };
    for (int i = 0; i < 8; i++) {
        XGrabKey(dpy, tab_key   = XKeysymToKeycode(dpy, XStringToKeysym("Tab")),   Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, up_key    = XKeysymToKeycode(dpy, XStringToKeysym("Up")),    Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, left_key  = XKeysymToKeycode(dpy, XStringToKeysym("Left")),  Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, right_key = XKeysymToKeycode(dpy, XStringToKeysym("Right")), Mod4Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, f4_key    = XKeysymToKeycode(dpy, XStringToKeysym("F4")),    Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, del_key   = XKeysymToKeycode(dpy, XStringToKeysym("Del")),   ControlMask|Mod1Mask|modifiers[i], root, True, GrabModeAsync, GrabModeAsync);
        XGrabButton(dpy, 1, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(dpy, 2, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(dpy, 3, Mod1Mask|modifiers[i], root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    }

    for(;;) {
        XNextEvent(dpy, &ev);
        // Keyboard keypress
        if (ev.type == KeyPress) {
            if (ev.xkey.keycode == tab_key) {
                XCirculateSubwindowsDown(dpy, root);
            }
            if (ev.xkey.subwindow != None) {
                if (ev.xkey.keycode == up_key) {
                    XGetWindowAttributes(dpy, root, &attr);
                    XMoveWindow(dpy, ev.xkey.subwindow, 0, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, attr.width, attr.height);
                }
                else if (ev.xkey.keycode == left_key) {
                    XGetWindowAttributes(dpy, root, &attr);
                    XMoveWindow(dpy, ev.xkey.subwindow, 0, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, attr.width / 2, attr.height);
                }
                else if (ev.xkey.keycode == right_key) {
                    XGetWindowAttributes(dpy, root, &attr);
                    XMoveWindow(dpy, ev.xkey.subwindow, attr.width / 2, 0);
                    XResizeWindow(dpy, ev.xkey.subwindow, attr.width / 2, attr.height);
                }
                else if (ev.xkey.keycode == del_key) {
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
                }
            }
        }
        // Mouse clicks
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            if (ev.xbutton.button == 3) XLowerWindow(dpy, ev.xbutton.subwindow);
            else XRaiseWindow(dpy, ev.xbutton.subwindow);
            XGrabPointer(dpy, ev.xbutton.subwindow, True, PointerMotionMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        else if (ev.type == ButtonRelease) {
            XUngrabPointer(dpy, CurrentTime);
        }
        // Mouse motion
        else if (ev.type == MotionNotify) {
            int xdiff, ydiff;
            while(XCheckTypedEvent(dpy, MotionNotify, &ev));
            xdiff = ev.xbutton.x_root - start.x_root;
            ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, ev.xmotion.window,
                attr.x + (start.button == 1 ? xdiff : 0),
                attr.y + (start.button == 1 ? ydiff : 0),
                MAX(1, attr.width + (start.button == 2 ? xdiff : 0)),
                MAX(1, attr.height + (start.button == 2 ? ydiff : 0)));
        }
        // Other events
        else if (ev.type == CreateNotify || ev.type == DestroyNotify || ev.type == CirculateNotify) {
            // I dont know, but if this happen on CreateNotify, just Google Chrome is afected with no mouse right-click feedback
            XSetInputFocus(dpy, root, None, CurrentTime);
        }
    }
}
