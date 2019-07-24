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

    if (!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, SubstructureNotifyMask|EnterWindowMask|LeaveWindowMask);

    // Intercept keys and mouse buttons. Mod1Mask = Alt. LockMask = CapsLock. Mod2Mask = NumLock
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Tab")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Tab")), Mod1Mask|LockMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Tab")), Mod1Mask|Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Tab")), Mod1Mask|Mod2Mask|LockMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 1, Mod1Mask|LockMask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 1, Mod1Mask|Mod2Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 1, Mod1Mask|Mod2Mask|LockMask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 2, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 2, Mod1Mask|LockMask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 2, Mod1Mask|Mod2Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 2, Mod1Mask|Mod2Mask|LockMask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    for(;;) {
        XNextEvent(dpy, &ev);
        // Keyboard keypress
        if (ev.type == KeyPress && ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("Tab"))) {
            XCirculateSubwindowsUp(dpy, root);
        }
        else if (ev.type == CirculateNotify) {
            XRaiseWindow(dpy, ev.xcirculate.window);
            XSetInputFocus(dpy, ev.xcirculate.window, None, CurrentTime);
        }
        else if ((ev.type == EnterNotify || ev.type == LeaveNotify) && ev.xcrossing.window != None) {
            XSetInputFocus(dpy, ev.xcrossing.window, None, CurrentTime);
        }
        // Mouse clicks
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            XGrabPointer(dpy, ev.xbutton.subwindow, True, PointerMotionMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
            if (ev.xbutton.button == 1 || ev.xbutton.button == 2) {
                XRaiseWindow(dpy, ev.xbutton.subwindow);
                XSetInputFocus(dpy, ev.xbutton.subwindow, None, CurrentTime);
            }
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
        else if (ev.type == ButtonRelease) {
            XUngrabPointer(dpy, CurrentTime);
        }
    }
}
