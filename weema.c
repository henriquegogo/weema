/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2025.
 * MIT License */

#include <X11/Xlib.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define NUMCODE(x) (x + 9)

Display *dpy = NULL;
XButtonEvent click_ev;
XWindowAttributes last_attr;
KeyCode enter_key, up_key, down_key, left_key, right_key, f4_key, del_key, tab_key;
Window root, last_win, owins[512];
int border = 1, margin = 0, panelheight = 0;

void SendEvent(Window win, const char *atom_name)
{
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(dpy, atom_name, False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, win, False, NoEventMask, &ev);
}

unsigned long GrayPixel(Display *dpy, int screen, int level)
{
    XColor color;
    color.red = color.green = color.blue = level * 655.35;
    color.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(dpy, XDefaultColormap(dpy, screen), &color);
    return color.pixel;
}

Window Clients(unsigned int iwin)
{
    unsigned int nwins, count = 1;
    XWindowAttributes wattr;
    Window win, *wins;
    XQueryTree(dpy, root, &win, &win, &wins, &nwins);
    panelheight = 0;

    for (int i = nwins - 1; i > 0; i--) {
        XGetWindowAttributes(dpy, wins[i], &wattr);
        int valid_window = wins[i] != None && !wattr.override_redirect && wattr.map_state == IsViewable;

        if (valid_window && wattr.height > 60 && iwin >= count++) {
            win = wins[i];
        } else if (valid_window && wattr.height <= 60) {
            panelheight = wattr.height;
        }
    }

    XFree(wins);
    return win;
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int mods)
{
    XWindowAttributes wattr, rattr;
    XGetWindowAttributes(dpy, win, &wattr);
    XGetWindowAttributes(dpy, root, &rattr);

    int top = 0 + margin, left = 0 + margin;
    int scr_width = wattr.screen->width - border * 2 - margin * 2;
    int scr_height = wattr.screen->height - border * 2 - margin * 2;
    int scnd_scr_width = rattr.width - scr_width - margin * 2;
    int scnd_scr_height = rattr.height - scr_height - margin * 2;

    if (wattr.x >= 0 && wattr.x < scr_width && wattr.y >= 0 && wattr.y < scr_height) {
        top = top + panelheight;
        scr_height = scr_height - panelheight;
    } else if (wattr.x < 0) {
        left = scnd_scr_width * -1 + margin - border * 2;
    } else if (wattr.x >= scr_width) {
        left = scnd_scr_width + margin - border * 2;
    } else if (wattr.y < 0) {
        top = scnd_scr_height * -1 + margin;
    } else if (wattr.y >= scr_height) {
        top = scnd_scr_height + margin;
    }

    int is_maximized = wattr.width == scr_width && wattr.height == scr_height && wattr.x == left && wattr.y == top;

    if (keycode == enter_key && wattr.x != left - margin && wattr.y != top - margin) {
        last_win = win;
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win,
            left - margin, 0, wattr.screen->width - border * 2, wattr.screen->height - border * 2);
    } else if (keycode == up_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MAX(wattr.height - scr_height / 4, scr_height / 4) - margin / 3);
    } else if (keycode == down_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, wattr.width, MIN(wattr.height + scr_height / 4, scr_height));
    } else if (keycode == left_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MAX(wattr.width - scr_width / 6, scr_width / 6) - margin / 3, wattr.height);
    } else if (keycode == right_key && mods & ShiftMask) {
        XResizeWindow(dpy, win, MIN(wattr.width + scr_width / 6, scr_width), wattr.height);
    } else if (keycode == up_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y - scr_height / 8);
    } else if (keycode == down_key && mods & ControlMask && is_maximized) {
        XMoveWindow(dpy, win, wattr.x, wattr.height + border * 2 + panelheight + margin * 3);
    } else if (keycode == down_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x, wattr.y + scr_height / 8);
    } else if (keycode == left_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x - scr_width / 12, wattr.y);
    } else if (keycode == right_key && mods & ControlMask && is_maximized) {
        XMoveWindow(dpy, win, wattr.width + border * 2 + margin * 3, wattr.y);
    } else if (keycode == right_key && mods & ControlMask) {
        XMoveWindow(dpy, win, wattr.x + scr_width / 12, wattr.y);
    } else if (win == last_win && (keycode == enter_key || (keycode == up_key && is_maximized) ||
            (keycode == down_key && (wattr.height == scr_height || wattr.height == scr_height + margin * 2)))) {
        XMoveResizeWindow(dpy, win, last_attr.x, last_attr.y, last_attr.width, last_attr.height);
    } else if (keycode == up_key && wattr.y == top) {
        last_win = win;
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left, top, scr_width, scr_height);
    } else if (keycode == up_key) {
        XMoveWindow(dpy, win, wattr.x, top);
    } else if (keycode == down_key) {
        XMoveWindow(dpy, win, wattr.x, scr_height - wattr.height + top);
    } else if (keycode == left_key && (wattr.x == left || is_maximized)) { 
        last_win = win;
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left, top, scr_width / 2 - margin / 2, scr_height);
    } else if (keycode == left_key) {
        XMoveWindow(dpy, win, left, wattr.y);
    } else if (keycode == right_key && ((wattr.x == scr_width - wattr.width + left) || is_maximized)) {
        last_win = win;
        last_attr = (XWindowAttributes){ .x = wattr.x, .y = wattr.y, .width = wattr.width, .height = wattr.height };
        XMoveResizeWindow(dpy, win, left + scr_width / 2 + margin / 2, top, scr_width / 2 - margin / 2, scr_height);
    } else if (keycode == right_key) {
        XMoveWindow(dpy, win, scr_width - wattr.width + left, wattr.y);
    }
}

void HandleNewWindow(Window win)
{
    Atom active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    Atom client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    XWindowAttributes wattr;
    XGetWindowAttributes(dpy, win, &wattr);
    int valid_window = win != None && !wattr.override_redirect && wattr.map_state == IsViewable;

    if (valid_window && wattr.height <= 60) {
        XMoveWindow(dpy, win, 0, 0);
        panelheight = wattr.height;
    } else if (valid_window) {
        for (int i = 0; i < 512; i++) {
            if (!owins[i]) {
                owins[i] = win;
                break;
            }
        }

        XSetWindowBorderWidth(dpy, win, border);
        XSetWindowBorder(dpy, win, GrayPixel(dpy, 0, 50));
        XSelectInput(dpy, win, FocusChangeMask);
        XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime); 
        XResizeWindow(dpy, win, wattr.width - 1, wattr.height);
        XChangeProperty(dpy, root, active_window, 33, 32, PropModeReplace, (unsigned char *) &(win), 1);
        XChangeProperty(dpy, root, client_list, 33, 32, PropModeReplace, (unsigned char *) owins, 512);

        if (wattr.x == 0 && wattr.y == 0) {
            XMoveWindow(dpy, win, wattr.x + margin, panelheight + margin);
        }
    }
}

void HandleMotion(XEvent ev)
{
    int xdiff = ev.xbutton.x_root - click_ev.x_root;
    int ydiff = ev.xbutton.y_root - click_ev.y_root;

    if (click_ev.button == Button1) {
        XMoveWindow(dpy, ev.xmotion.window, last_attr.x + xdiff, last_attr.y + ydiff);
    } else if (click_ev.button == Button3) {
        XResizeWindow(dpy, ev.xmotion.window, abs(last_attr.width + xdiff) + 1, abs(last_attr.height + ydiff) + 1);
    }
}

void GrabKey(int keycode, unsigned int mods)
{
    XGrabKey(dpy, keycode, mods, root, True, GrabModeAsync, GrabModeAsync);
}

KeyCode GetKeycode(const char *key)
{
    return XKeysymToKeycode(dpy, XStringToKeysym(key));
}

void SetupGrab()
{
    unsigned int mods[8] = {
        None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask
    };

    for (int i = 0; i < 8; i++) {
        for (unsigned int ikey = 1; ikey <= 9; ikey++) {
            GrabKey(NUMCODE(ikey), Mod4Mask|mods[i]);
        }

        GrabKey(enter_key = GetKeycode("Return"), Mod4Mask|mods[i]);
        GrabKey(up_key    = GetKeycode("Up"),     Mod4Mask|mods[i]);
        GrabKey(down_key  = GetKeycode("Down"),   Mod4Mask|mods[i]);
        GrabKey(left_key  = GetKeycode("Left"),   Mod4Mask|mods[i]);
        GrabKey(right_key = GetKeycode("Right"),  Mod4Mask|mods[i]);
        GrabKey(f4_key    = GetKeycode("F4"),     Mod1Mask|mods[i]);
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

        XGrabButton(dpy, AnyButton, Mod4Mask|mods[i], root, True, ButtonPressMask, 1, 1, None, None);
    }
}

void InterceptEvents()
{
    XEvent ev;
    XNextEvent(dpy, &ev);

    int revert_to, keycode = ev.xkey.keycode, state = ev.xkey.state;
    Atom active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    Atom client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    Window focused;
    XGetInputFocus(dpy, &focused, &revert_to);

    if (ev.type == KeyPress && keycode >= NUMCODE(1) && keycode <= NUMCODE(9) && owins[keycode - 10]) {
        XSetInputFocus(dpy, owins[keycode - 10], RevertToPointerRoot, CurrentTime); 
    } else if (ev.type == KeyPress && keycode == tab_key && state & Mod1Mask) {
        XSetInputFocus(dpy, Clients(2), RevertToPointerRoot, CurrentTime); 
    } else if (ev.type == KeyPress && keycode == tab_key && state & Mod4Mask) {
        int li = 0;
        int fi = 0;

        for (; owins[li]; li++, fi++) {
            if (owins[fi] == focused) fi--;
        }

        Window win = li ? owins[state & ShiftMask ? (fi - 1 + li) % li : (fi + 1) % li] : None;
        XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime); 
    } else if (ev.type == KeyPress && keycode == del_key) {
        XCloseDisplay(dpy);
    } else if (ev.type == KeyPress && keycode == f4_key) {
        SendEvent(focused, "WM_DELETE_WINDOW");
    } else if (ev.type == KeyPress) {
        HandleWindowPosition(focused, keycode, state);
    } else if (ev.type == ButtonPress && ev.xbutton.window != root) {
        XSetInputFocus(dpy, ev.xbutton.window, RevertToPointerRoot, CurrentTime); 
    } else if (ev.type == ButtonPress && ev.xbutton.window == root &&
            ev.xbutton.subwindow != None && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
        XSetInputFocus(dpy, (click_ev = ev.xbutton).subwindow, RevertToPointerRoot, CurrentTime); 
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
        for (int i = 0, j = i; i < 511; i++, j++) {
            owins[i] = owins[i] == ev.xunmap.window ? owins[++j] : owins[j];
        }

        XSetInputFocus(dpy, Clients(1), RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, client_list, 33, 32, PropModeReplace, (unsigned char *) owins, 512);
    } else if (ev.type == FocusIn && ev.xfocus.detail != NotifyPointer && ev.xfocus.detail != NotifyPointerRoot) {
        XRaiseWindow(dpy, ev.xfocus.window);
        XSetWindowBorder(dpy, ev.xfocus.window, GrayPixel(dpy, 0, 50));
        XUngrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        XChangeProperty(dpy, root, active_window, 33, 32, PropModeReplace, (unsigned char *) &(ev.xfocus.window), 1);
    } else if (ev.type == FocusOut && ev.xfocus.detail != NotifyPointer && ev.xfocus.detail != NotifyPointerRoot) {
        XSetWindowBorder(dpy, ev.xfocus.window, BlackPixel(dpy, 0));
        XGrabButton(dpy, AnyButton, AnyModifier, ev.xfocus.window, True, ButtonPressMask, 0, 0, None, None);
    } else if (ev.xclient.message_type == active_window) {
        XSetInputFocus(dpy, ev.xclient.window, RevertToPointerRoot, CurrentTime); 
    }
}

int ErrorHandler()
{
    return 0;
}

int main()
{
    if (!(dpy = XOpenDisplay(NULL)) || !(root = XDefaultRootWindow(dpy)))
        return 1;

    if (getenv("WEEMA_BORDER"))
        border = atoi(getenv("WEEMA_BORDER"));

    if (getenv("WEEMA_MARGIN"))
        margin = atoi(getenv("WEEMA_MARGIN"));

    XSetErrorHandler(ErrorHandler);
    XSelectInput(dpy, root, SubstructureNotifyMask);
    XDefineCursor(dpy, root, XCreateFontCursor(dpy, 2));
    SetupGrab(); 

    Atom supported, atoms[] = {
        supported = XInternAtom(dpy, "_NET_SUPPORTED", False),
        XInternAtom(dpy, "_NET_CLIENT_LIST", False),
        XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False),
    };

    XChangeProperty(dpy, root, supported, 4, 32, PropModeReplace, (unsigned char*)atoms, sizeof(atoms) / sizeof(Atom));

    for(;;) InterceptEvents();
}
