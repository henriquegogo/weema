/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2021.
 * MIT License */

#include <stdlib.h>
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>

typedef struct {
    XButtonEvent ev;
    XWindowAttributes attr;
} Clicked;
Clicked clicked;

Display *display = NULL;
int current_screen_width;
int current_screen_height;
int top = 0, left = 0;

KeyCode up_key, down_key, left_key, right_key,
        r_key, t_key, l_key, b_key, w_key,
        vol_up_key, vol_down_key,
        f4_key, del_key, tab_key, print_key;

void SetupScreen(XWindowAttributes win_attr) {
    Screen *screen = XDefaultScreenOfDisplay(display);
    int default_screen_width = XWidthOfScreen(screen);
    int default_screen_height = XHeightOfScreen(screen);

    if (win_attr.x >= default_screen_width) {
        XWindowAttributes root_attr;
        XGetWindowAttributes(display, XDefaultRootWindow(display), &root_attr);

        left = default_screen_width;
        current_screen_width = root_attr.width - default_screen_width;

        if (root_attr.height > default_screen_height) {
            current_screen_height = root_attr.height;
        }
        else {
            current_screen_height = default_screen_height;
        }
    }
    else {
        left = 0;
        current_screen_width = default_screen_width;
        current_screen_height = default_screen_height;
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

void PositionUp(Window win, XWindowAttributes win_attr) {
    int center_pos = (current_screen_height - win_attr.height) / 2;
    Bool at_top = win_attr.y == top;

    if (at_top) {
        XMoveResizeWindow(display, win, left, top, current_screen_width, current_screen_height - top);
    }
    else if (win_attr.y > center_pos) {
        XMoveWindow(display, win, win_attr.x, center_pos);
    }
    else {
        XMoveWindow(display, win, win_attr.x, top);
    }
}

void PositionDown(Window win, XWindowAttributes win_attr) {
    int center_pos = (current_screen_height - win_attr.height) / 2;
    Bool is_full = win_attr.width == current_screen_width && win_attr.height == current_screen_height - top;

    if (is_full) {
        XMoveResizeWindow(display, win, current_screen_width / 3 / 2 + left, current_screen_height / 3 / 2,
                current_screen_width / 3 * 2, current_screen_height / 3 * 2);
    }
    else if (win_attr.y < center_pos) {
        XMoveWindow(display, win, win_attr.x, center_pos);
    }
    else {
        XMoveWindow(display, win, win_attr.x, current_screen_height - win_attr.height);
    }
}

void PositionLeft(Window win, XWindowAttributes win_attr) {
    int center_pos = (current_screen_width - win_attr.width) / 2;
    Bool at_left = win_attr.x == left;

    if (at_left) {
        XMoveResizeWindow(display, win, left, top, win_attr.width, current_screen_height - top);
    }
    else if (win_attr.x > (center_pos + left)) {
        XMoveWindow(display, win, center_pos + left, win_attr.y);
    }
    else {
        XMoveWindow(display, win, left, win_attr.y);
    }
}

void PositionRight(Window win, XWindowAttributes win_attr) {
    int center_pos = (current_screen_width - win_attr.width) / 2;
    int right_pos = current_screen_width - win_attr.width + left;
    Bool at_right = win_attr.x == right_pos;

    if (at_right) {
        XMoveResizeWindow(display, win, right_pos, top, win_attr.width, current_screen_height - top);
    }
    else if (win_attr.x < (center_pos + left)) {
        XMoveWindow(display, win, center_pos + left, win_attr.y);
    }
    else {
        XMoveWindow(display, win, current_screen_width - win_attr.width + left, win_attr.y);
    }
}

void ResizeUp(Window win, XWindowAttributes win_attr) {
    if (win_attr.height <= current_screen_height / 3 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 4 - top);
    }
    else if (win_attr.height <= current_screen_height / 2 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 3 - top);
    }
    else if (win_attr.height <= 2 * current_screen_height / 3 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 2 - top);
    }
    else if (win_attr.height <= 3 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, 2 * current_screen_height / 3 - top);
    }
    else if (win_attr.height <= current_screen_height - top) {
        XResizeWindow(display, win, win_attr.width, 3 * current_screen_height / 4 - top);
    }
}

void ResizeDown(Window win, XWindowAttributes win_attr) {
    if (win_attr.height < current_screen_height / 3 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 3 - top);
    }
    else if (win_attr.height < current_screen_height / 2 - top) {
        XResizeWindow(display, win, win_attr.width, current_screen_height / 2 - top);
    }
    else if (win_attr.height < 2 * current_screen_height / 3 - top) {
        XResizeWindow(display, win, win_attr.width, 2 * current_screen_height / 3 - top);
    }
    else if (win_attr.height < 3 * current_screen_height / 4 - top) {
        XResizeWindow(display, win, win_attr.width, 3 * current_screen_height / 4 - top);
    }
    else {
        XMoveResizeWindow(display, win, win_attr.x, top, win_attr.width, current_screen_height - top);
    }
}

void ResizeLeft(Window win, XWindowAttributes win_attr) {
    if (win_attr.width <= current_screen_width / 3) {
        XResizeWindow(display, win, current_screen_width / 4, win_attr.height);
    }
    else if (win_attr.width <= current_screen_width / 2) {
        XResizeWindow(display, win, current_screen_width / 3, win_attr.height);
    }
    else if (win_attr.width <= 2 * current_screen_width / 3) {
        XResizeWindow(display, win, current_screen_width / 2, win_attr.height);
    }
    else if (win_attr.width <= 3 * current_screen_width / 4) {
        XResizeWindow(display, win, 2 * current_screen_width / 3, win_attr.height);
    }
    else if (win_attr.width <= current_screen_width) {
        XResizeWindow(display, win, 3 * current_screen_width / 4, win_attr.height);
    }
}

void ResizeRight(Window win, XWindowAttributes win_attr) {
    if (win_attr.width < current_screen_width / 3) {
        XResizeWindow(display, win, current_screen_width / 3, win_attr.height);
    }
    else if (win_attr.width < current_screen_width / 2) {
        XResizeWindow(display, win, current_screen_width / 2, win_attr.height);
    }
    else if (win_attr.width < 2 * current_screen_width / 3) {
        XResizeWindow(display, win, 2 * current_screen_width / 3, win_attr.height);
    }
    else if (win_attr.width < 3 * current_screen_width / 4) {
        XResizeWindow(display, win, 3 * current_screen_width / 4, win_attr.height);
    }
    else {
        XMoveResizeWindow(display, win, left, win_attr.y, current_screen_width, win_attr.height);
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

    if (clicked.ev.button == Button1) {
        XMoveWindow(display, ev.xmotion.window, clicked.attr.x + xdiff, clicked.attr.y + ydiff);
    }
    else if (clicked.ev.button == Button2) {
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
        XSelectInput(display, win, EnterWindowMask);
    }
}

Window FocusedWindow() {
    Window focused_win;
    int revert;
    XGetInputFocus(display, &focused_win, &revert);

    return focused_win;
}

Window GetWindow(int win_i) {
    unsigned int i, nwins;
    int count = 1;
    Window win, last_but_one, *wins;
    XWindowAttributes win_attr;

    XQueryTree(display, XDefaultRootWindow(display), &win, &win, &wins, &nwins);

    for (i = 0; i < nwins; i++) {
        XGetWindowAttributes(display, wins[i], &win_attr);

        if (wins[i] != None && !win_attr.override_redirect && win_attr.map_state == IsViewable) {
            win = wins[i];
            if (win_i == count++ || i == nwins - 1) break;
            last_but_one = win;
        }
    }

    XFree(wins);

    return win_i == -1 ? last_but_one : win;
}

void HandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);
    SetupScreen(win_attr);

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
    else if (keycode == up_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x, win_attr.y - 100);
    }
    else if (keycode == down_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x, win_attr.y + 100);
    }
    else if (keycode == left_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x - 100, win_attr.y);
    }
    else if (keycode == right_key && modifiers & Mod1Mask) {
        XMoveWindow(display, win, win_attr.x + 100, win_attr.y);
    }
    else if (keycode == up_key) {
        PositionUp(win, win_attr);
    }
    else if (keycode == down_key) {
        PositionDown(win, win_attr);
    }
    else if (keycode == left_key) {
        PositionLeft(win, win_attr);
    }
    else if (keycode == right_key) {
        PositionRight(win, win_attr);
    }

    XRaiseWindow(display, win);
    XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress && (ev.xbutton.button == Button1 || ev.xbutton.button == Button2)
            && ev.xbutton.subwindow != None) {
        XRaiseWindow(display, ev.xbutton.subwindow);
        XSetInputFocus(display, ev.xbutton.subwindow, RevertToPointerRoot, CurrentTime); 
        HandleClick(ev.xbutton);
    }
    else if (ev.type == ButtonPress && ev.xbutton.button == Button3 && ev.xbutton.subwindow != None) {
        XLowerWindow(display, ev.xbutton.subwindow);
    }
    else if (ev.type == ButtonRelease) {
        XUngrabPointer(display, CurrentTime);
    }
    else if (ev.type == MotionNotify) {
        HandleMotion(ev);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == r_key) {
        RunCmd("dmenu_run", "$WEEMA_LAUNCHER");
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
        Window win = GetWindow(1);
        XRaiseWindow(display, FocusedWindow());
        XRaiseWindow(display, win);
        XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        Window win = GetWindow(-1);
        XRaiseWindow(display, FocusedWindow());
        XRaiseWindow(display, win);
        XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(display);
        exit(0);
    }
    else if (ev.type == KeyPress && (ev.xkey.keycode == f4_key || ev.xkey.keycode == w_key)) {
        CloseWindow(FocusedWindow());
    }
    else if (ev.type == KeyPress) {
        HandleWindowPosition(FocusedWindow(), ev.xkey.keycode, ev.xkey.state);
    }
    else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    }
    else if (ev.type == UnmapNotify) {
        XSetInputFocus(display, GetWindow(0), RevertToPointerRoot, CurrentTime);
    }
    else if (ev.type == EnterNotify) {
        XSetInputFocus(display, ev.xcrossing.window, RevertToPointerRoot, CurrentTime); 
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

    RunCmd("xsetroot -cursor_name arrow -solid \"#030609\"", NULL);
    RunCmd("feh --bg-scale ~/wallpaper.jpg", "$WEEMA_INIT");

    for(;;) {
        InterceptEvents();
    }
}
