/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

Display *display = NULL;

typedef struct {
    Window win;
    XWindowAttributes attr;
    int half_w;
    int third_w;
    int quarter_w;
    int half_h;
    int third_h;
    int quarter_h;
} Root;
Root root;

typedef struct {
    XButtonEvent ev;
    XWindowAttributes attr;
} Clicked;
Clicked clicked;

KeyCode up_key, down_key, left_key, right_key,
        r_key, t_key, l_key, b_key,
        vol_up_key, vol_down_key,
        f4_key, del_key, tab_key, print_key;

void InitRootWindow() {
    root.win = XDefaultRootWindow(display);
    XSelectInput(display, root.win, SubstructureNotifyMask|FocusChangeMask);
    XGetWindowAttributes(display, root.win, &root.attr);

    root.half_w    = root.attr.width / 2;
    root.third_w   = root.attr.width / 3;
    root.quarter_w = root.attr.width / 4;
    root.half_h    = root.attr.height / 2;
    root.third_h   = root.attr.height / 3;
    root.quarter_h = root.attr.height / 4;
}

void GrabKey(int keycode, unsigned int modifiers) {
    XGrabKey(display, keycode, modifiers, root.win, True, GrabModeAsync, GrabModeAsync);
}

void GrabButton(int buttoncode, unsigned int modifiers) {
    XGrabButton(display, buttoncode, modifiers, root.win, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

int GetKeycode(const char *key) {
    return XKeysymToKeycode(display, XStringToKeysym(key));
}

void SetupGrab() {
    // Intercept keys and mouse buttons. Mod2Mask=NumLock, Mod3Mask=ScrollLock, LockMask=CapsLock
    unsigned int modifiers[8] = { None, Mod2Mask, Mod3Mask, LockMask,
        Mod2Mask|Mod3Mask, Mod2Mask|LockMask, Mod3Mask|LockMask, Mod2Mask|Mod3Mask|LockMask };

    for (int i = 0; i < 8; i++) {
        GrabKey(vol_up_key   = GetKeycode("XF86AudioRaiseVolume"), modifiers[i]);
        GrabKey(vol_down_key = GetKeycode("XF86AudioLowerVolume"), modifiers[i]);
        GrabKey(r_key     = GetKeycode("r"),     Mod4Mask|modifiers[i]);
        GrabKey(t_key     = GetKeycode("t"),     Mod4Mask|modifiers[i]);
        GrabKey(l_key     = GetKeycode("l"),     Mod4Mask|modifiers[i]);
        GrabKey(b_key     = GetKeycode("b"),     Mod4Mask|modifiers[i]);
        GrabKey(print_key = GetKeycode("Print"), modifiers[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),   Mod1Mask|modifiers[i]);
        GrabKey(tab_key   = GetKeycode("Tab"),   ShiftMask|Mod1Mask|modifiers[i]);
        GrabKey(f4_key    = GetKeycode("F4"),    Mod1Mask|modifiers[i]);
        GrabKey(del_key   = GetKeycode("Delete"),ControlMask|Mod1Mask|modifiers[i]);
        GrabKey(up_key    = GetKeycode("Up"),    Mod4Mask|modifiers[i]);
        GrabKey(down_key  = GetKeycode("Down"),  Mod4Mask|modifiers[i]);
        GrabKey(left_key  = GetKeycode("Left"),  Mod4Mask|modifiers[i]);
        GrabKey(right_key = GetKeycode("Right"), Mod4Mask|modifiers[i]);
        GrabKey(up_key    = GetKeycode("Up"),    ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(down_key  = GetKeycode("Down"),  ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(left_key  = GetKeycode("Left"),  ShiftMask|Mod4Mask|modifiers[i]);
        GrabKey(right_key = GetKeycode("Right"), ShiftMask|Mod4Mask|modifiers[i]);
        GrabButton(AnyButton, Mod1Mask|modifiers[i]);
    }
}

void RunCmd(char *cmd, char *env_var) {
    char system_cmd[512];

    if (env_var != NULL) {
        sprintf(system_cmd, "if [ \"%s\" ]; then sh -c \"%s &\"; else sh -c \"%s &\"; fi",
                env_var, env_var, cmd);
    }
    else {
        sprintf(system_cmd, "%s", cmd);
    }

    (void)(system(system_cmd)+1);
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

void MoveUp(Window win, XWindowAttributes win_attr) {
    int center_position = (root.attr.height - win_attr.height) / 2;
    Bool at_top = win_attr.y == 0;

    if (at_top) {
        XMoveResizeWindow(display, win, 0, 0, root.attr.width, root.attr.height);
    }
    else if (win_attr.y > center_position) {
        XMoveWindow(display, win, win_attr.x, center_position);
    }
    else {
        XMoveWindow(display, win, win_attr.x, 0);
    }
}

void MoveDown(Window win, XWindowAttributes win_attr) {
    int center_position = (root.attr.height - win_attr.height) / 2;
    Bool is_fullscreen = win_attr.width == root.attr.width && win_attr.height == root.attr.height;

    if (is_fullscreen) {
        XMoveResizeWindow(display, win, root.third_w / 2 + 1, root.third_h / 2,
                root.third_w * 2, root.third_h * 2);
    }
    else if (win_attr.y < center_position) {
        XMoveWindow(display, win, win_attr.x, center_position);
    }
    else {
        XMoveWindow(display, win, win_attr.x, root.attr.height - win_attr.height);
    }
}

void MoveLeft(Window win, XWindowAttributes win_attr) {
    int center_position = (root.attr.width - win_attr.width) / 2;
    Bool at_left = win_attr.x == 0;

    if (at_left) {
        XMoveResizeWindow(display, win, 0, 0, win_attr.width, root.attr.height);
    }
    else if (win_attr.x > center_position) {
        XMoveWindow(display, win, center_position, win_attr.y);
    }
    else {
        XMoveWindow(display, win, 0, win_attr.y);
    }
}

void MoveRight(Window win, XWindowAttributes win_attr) {
    int center_position = (root.attr.width - win_attr.width) / 2;
    int right_position = root.attr.width - win_attr.width;
    Bool at_right = win_attr.x == root.attr.width - win_attr.width;

    if (at_right) {
        XMoveResizeWindow(display, win, right_position, 0, win_attr.width, root.attr.height);
    }
    else if (win_attr.x < center_position) {
        XMoveWindow(display, win, center_position, win_attr.y);
    }
    else {
        XMoveWindow(display, win, root.attr.width - win_attr.width, win_attr.y);
    }
}

void ResizeUp(Window win, XWindowAttributes win_attr) {
    if (win_attr.height <= root.third_h) {
        XResizeWindow(display, win, win_attr.width, root.quarter_h);
    }
    else if (win_attr.height <= root.half_h) {
        XResizeWindow(display, win, win_attr.width, root.third_h);
    }
    else if (win_attr.height <= 2 * root.third_h) {
        XResizeWindow(display, win, win_attr.width, root.half_h);
    }
    else if (win_attr.height <= 3 * root.quarter_h) {
        XResizeWindow(display, win, win_attr.width, 2 * root.third_h);
    }
    else if (win_attr.height <= root.attr.height) {
        XResizeWindow(display, win, win_attr.width, 3 * root.quarter_h);
    }
}

void ResizeDown(Window win, XWindowAttributes win_attr) {
    if (win_attr.height < root.third_h) {
        XResizeWindow(display, win, win_attr.width, root.third_h);
    }
    else if (win_attr.height < root.half_h) {
        XResizeWindow(display, win, win_attr.width, root.half_h);
    }
    else if (win_attr.height < 2 * root.third_h) {
        XResizeWindow(display, win, win_attr.width, 2 * root.third_h);
    }
    else if (win_attr.height < 3 * root.quarter_h) {
        XResizeWindow(display, win, win_attr.width, 3 * root.quarter_h);
    }
    else {
        XMoveResizeWindow(display, win, win_attr.x, 0, win_attr.width, root.attr.height);
    }
}

void ResizeLeft(Window win, XWindowAttributes win_attr) {
    if (win_attr.width <= root.third_w) {
        XResizeWindow(display, win, root.quarter_w, win_attr.height);
    }
    else if (win_attr.width <= root.half_w) {
        XResizeWindow(display, win, root.third_w, win_attr.height);
    }
    else if (win_attr.width <= 2 * root.third_w) {
        XResizeWindow(display, win, root.half_w, win_attr.height);
    }
    else if (win_attr.width <= 3 * root.quarter_w) {
        XResizeWindow(display, win, 2 * root.third_w, win_attr.height);
    }
    else if (win_attr.width <= root.attr.width) {
        XResizeWindow(display, win, 3 * root.quarter_w, win_attr.height);
    }
}

void ResizeRight(Window win, XWindowAttributes win_attr) {
    if (win_attr.width < root.third_w) {
        XResizeWindow(display, win, root.third_w, win_attr.height);
    }
    else if (win_attr.width < root.half_w) {
        XResizeWindow(display, win, root.half_w, win_attr.height);
    }
    else if (win_attr.width < 2 * root.third_w) {
        XResizeWindow(display, win, 2 * root.third_w, win_attr.height);
    }
    else if (win_attr.width < 3 * root.quarter_w) {
        XResizeWindow(display, win, 3 * root.quarter_w, win_attr.height);
    }
    else {
        XMoveResizeWindow(display, win, 0, win_attr.y, root.attr.width, win_attr.height);
    }
}

void HandleClick(XButtonEvent button_event) {
    XGrabPointer(display, button_event.subwindow, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(display, button_event.subwindow, &clicked.attr);
    clicked.ev = button_event;
}

void HandleMotion(XEvent ev) {
    while (XCheckTypedEvent(display, MotionNotify, &ev));

    int xdiff = ev.xbutton.x_root - clicked.ev.x_root;
    int ydiff = ev.xbutton.y_root - clicked.ev.y_root;

    if (clicked.ev.button == 1) {
        XMoveWindow(display, ev.xmotion.window, clicked.attr.x + xdiff, clicked.attr.y + ydiff);
    }
    else if (clicked.ev.button == 2) {
        XResizeWindow(display, ev.xmotion.window,
                abs(clicked.attr.width + xdiff) + 1, abs(clicked.attr.height + ydiff) + 1);
    }
}

void HandleNewWindow(Window win) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);

    if (win != None && !win_attr.override_redirect && win_attr.map_state == IsViewable) {
        XSetWindowBorderWidth(display, win, 1);
        XSetWindowBorder(display, win, 0);
        XRaiseWindow(display, win);
        XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
    }
}

Window GetWindow(unsigned int win_i) {
    unsigned int i, nwins, count = 1;
    Window win, *wins;
    XWindowAttributes win_attr;

    XQueryTree(display, root.win, &win, &win, &wins, &nwins);

    for (i = 0; i < nwins; i++) {
        XGetWindowAttributes(display, wins[i], &win_attr);

        if (wins[i] != None && !win_attr.override_redirect && win_attr.map_state == IsViewable) {
            win = wins[i];
            if (win_i == count++) break;
        }
    }

    XFree(wins);

    return win;
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);

    if (keycode == up_key && modifiers & ShiftMask) {
        ResizeUp(win, win_attr);
    }
    else if (keycode == down_key && modifiers & ShiftMask) {
        ResizeDown(win, win_attr);
    }
    else if (keycode == left_key && modifiers & ShiftMask) {
        ResizeLeft(win, win_attr);
    }
    else if (keycode == right_key && modifiers & ShiftMask) {
        ResizeRight(win, win_attr);
    }
    else if (keycode == up_key) {
        MoveUp(win, win_attr);
    }
    else if (keycode == down_key) {
        MoveDown(win, win_attr);
    }
    else if (keycode == left_key) {
        MoveLeft(win, win_attr);
    }
    else if (keycode == right_key) {
        MoveRight(win, win_attr);
    }

    XRaiseWindow(display, win);
    XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress && ev.xbutton.button == 3 && ev.xbutton.subwindow != None) {
        XLowerWindow(display, ev.xbutton.subwindow);
        XSetInputFocus(display, GetWindow(0), RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == ButtonPress && ev.xbutton.subwindow != None) {
        XRaiseWindow(display, ev.xbutton.subwindow);
        XSetInputFocus(display, ev.xbutton.subwindow, RevertToPointerRoot, CurrentTime); 
        HandleClick(ev.xbutton);
    }
    else if (ev.type == ButtonRelease) {
        XUngrabPointer(display, CurrentTime);
    }
    else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == r_key) {
        RunCmd("dmenu_run -l 5 -p '$(date +'%d %a %H:%M')'", "$WEEMA_LAUNCHER");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
        RunCmd("x-terminal-emulator", "$WEEMA_TERMINAL");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == b_key) {
        RunCmd("x-www-browser", "$WEEMA_BROWSER");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == l_key) {
        RunCmd("slock", "$WEEMA_LOCK");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
        RunCmd("amixer set Master 3+", "$WEEMA_VOLUMEUP");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
        RunCmd("amixer set Master 3-", "$WEEMA_VOLUMEDOWN");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
        RunCmd("scrot", "$WEEMA_PRINTSCREEN");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key && ev.xkey.state & ShiftMask) {
        XLowerWindow(display, GetWindow(0));
        XSetInputFocus(display, GetWindow(0), RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        Window win = GetWindow(1);
        XRaiseWindow(display, win);
        XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(display);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == f4_key) {
        CloseWindow(GetWindow(0));
    }
    else if (ev.type == KeyPress) {
        HandleWindowPosition(GetWindow(0), ev.xkey.keycode, ev.xkey.state);
    }
    else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    }
    else if (ev.type == FocusIn && ev.xfocus.window == root.win) {
        XSetInputFocus(display, GetWindow(0), RevertToPointerRoot, CurrentTime); 
    }
}

int main() {
    if (!(display = XOpenDisplay(0x0))) return 1;

    InitRootWindow();
    SetupGrab();

    RunCmd("xsetroot -cursor_name arrow -solid \"#030609\"", NULL);
    RunCmd("feh --bg-scale ~/wallpaper.jpg", "$WEEMA_INIT");
        
    for(;;) {
        InterceptEvents();
    }
}
