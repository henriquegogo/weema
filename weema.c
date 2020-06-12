/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

Display *display = NULL;
Window root_win;
XWindowAttributes root_attr, win_attr;
XButtonEvent click_start;
XEvent ev;

KeyCode up_key, down_key, left_key, right_key,
        r_key, t_key, l_key, b_key,
        vol_up_key, vol_down_key,
        f4_key, del_key, tab_key, print_key;

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
        WeeGrabKey(b_key     = WeeGetKeycode("b"),     Mod4Mask|modifiers[i]);
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

void WeeDrawBorder(Window win) {
    XSetWindowBorderWidth(display, win, 1);
    XSetWindowBorder(display, win, 0);
}

void WeeMoveCursor(Window win, unsigned int width, unsigned int height) {
    XWarpPointer(display, None, win, None, None, None, None, width, height);
}

void WeeCenterCursor(Window win) {
    XGetWindowAttributes(display, win, &win_attr);
    WeeMoveCursor(win, win_attr.width / 2, win_attr.height / 2);
}

void WeeMoveUp(Window win) {
    int center_position = (root_attr.height - win_attr.height) / 2;
    Bool at_top = win_attr.y == 0;

    if (at_top) {
        XMoveResizeWindow(display, win, 0, 0, root_attr.width, root_attr.height);
    }
    else if (win_attr.y > center_position) {
        XMoveWindow(display, win, win_attr.x, center_position);
    }
    else {
        XMoveWindow(display, win, win_attr.x, 0);
    }
}

void WeeMoveDown(Window win) {
    int third_width = root_attr.width / 3;
    int third_height = root_attr.height / 3;
    int center_position = (root_attr.height - win_attr.height) / 2;
    Bool is_fullscreen = win_attr.width == root_attr.width && win_attr.height == root_attr.height;

    if (is_fullscreen) {
        XMoveResizeWindow(display, win, third_width / 2 + 1, third_height / 2, third_width * 2, third_height * 2);
    }
    else if (win_attr.y < center_position) {
        XMoveWindow(display, win, win_attr.x, center_position);
    }
    else {
        XMoveWindow(display, win, win_attr.x, root_attr.height - win_attr.height);
    }
}

void WeeMoveLeft(Window win) {
    int center_position = (root_attr.width - win_attr.width) / 2;
    Bool at_left = win_attr.x == 0;

    if (at_left) {
        XMoveResizeWindow(display, win, 0, 0, win_attr.width, root_attr.height);
    }
    else if (win_attr.x > center_position) {
        XMoveWindow(display, win, center_position, win_attr.y);
    }
    else {
        XMoveWindow(display, win, 0, win_attr.y);
    }
}

void WeeMoveRight(Window win) {
    int center_position = (root_attr.width - win_attr.width) / 2;
    int right_position = root_attr.width - win_attr.width;
    Bool at_right = win_attr.x == root_attr.width - win_attr.width;

    if (at_right) {
        XMoveResizeWindow(display, win, right_position, 0, win_attr.width, root_attr.height);
    }
    else if (win_attr.x < center_position) {
        XMoveWindow(display, win, center_position, win_attr.y);
    }
    else {
        XMoveWindow(display, win, root_attr.width - win_attr.width, win_attr.y);
    }
}

void WeeResizeUp(Window win) {
    int half = root_attr.height / 2;
    int third = root_attr.height / 3;
    int quarter = root_attr.height / 4;

    if (win_attr.height <= third) {
        XResizeWindow(display, win, win_attr.width, quarter);
    }
    else if (win_attr.height <= half) {
        XResizeWindow(display, win, win_attr.width, third);
    }
    else if (win_attr.height <= 2 * third) {
        XResizeWindow(display, win, win_attr.width, half);
    }
    else if (win_attr.height <= 3 * quarter) {
        XResizeWindow(display, win, win_attr.width, 2 * third);
    }
    else if (win_attr.height <= root_attr.height) {
        XResizeWindow(display, win, win_attr.width, 3 * quarter);
    }
}

void WeeResizeDown(Window win) {
    int half = root_attr.height / 2;
    int third = root_attr.height / 3;
    int quarter = root_attr.height / 4;

    if (win_attr.height < third) {
        XResizeWindow(display, win, win_attr.width, third);
    }
    else if (win_attr.height < half) {
        XResizeWindow(display, win, win_attr.width, half);
    }
    else if (win_attr.height < 2 * third) {
        XResizeWindow(display, win, win_attr.width, 2 * third);
    }
    else if (win_attr.height < 3 * quarter) {
        XResizeWindow(display, win, win_attr.width, 3 * quarter);
    }
    else {
        XMoveResizeWindow(display, win, win_attr.x, 0, win_attr.width, root_attr.height);
    }
}

void WeeResizeLeft(Window win) {
    int half = root_attr.width / 2;
    int third = root_attr.width / 3;
    int quarter = root_attr.width / 4;

    if (win_attr.width <= third) {
        XResizeWindow(display, win, quarter, win_attr.height);
    }
    else if (win_attr.width <= half) {
        XResizeWindow(display, win, third, win_attr.height);
    }
    else if (win_attr.width <= 2 * third) {
        XResizeWindow(display, win, half, win_attr.height);
    }
    else if (win_attr.width <= 3 * quarter) {
        XResizeWindow(display, win, 2 * third, win_attr.height);
    }
    else if (win_attr.width <= root_attr.width) {
        XResizeWindow(display, win, 3 * quarter, win_attr.height);
    }
}

void WeeResizeRight(Window win) {
    int half = root_attr.width / 2;
    int third = root_attr.width / 3;
    int quarter = root_attr.width / 4;

    if (win_attr.width < third) {
        XResizeWindow(display, win, third, win_attr.height);
    }
    else if (win_attr.width < half) {
        XResizeWindow(display, win, half, win_attr.height);
    }
    else if (win_attr.width < 2 * third) {
        XResizeWindow(display, win, 2 * third, win_attr.height);
    }
    else if (win_attr.width < 3 * quarter) {
        XResizeWindow(display, win, 3 * quarter, win_attr.height);
    }
    else {
        XMoveResizeWindow(display, win, 0, win_attr.y, root_attr.width, win_attr.height);
    }
}

void WeeRaiseAndFocus(Window win) {
    XRaiseWindow(display, win);
    XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
}

void WeeHandleWindowPosition(Window win, unsigned int keycode, unsigned int modifiers) {
    XGetWindowAttributes(display, win, &win_attr);

    if (keycode == up_key && modifiers & ShiftMask) {
        WeeResizeUp(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == down_key && modifiers & ShiftMask) {
        WeeResizeDown(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == left_key && modifiers & ShiftMask) {
        WeeResizeLeft(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == right_key && modifiers & ShiftMask) {
        WeeResizeRight(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == up_key) {
        WeeMoveUp(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == down_key) {
        WeeMoveDown(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == left_key) {
        WeeMoveLeft(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
    else if (keycode == right_key) {
        WeeMoveRight(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
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

void WeeHandleNewWindow(Window win) {
    XGetWindowAttributes(display, win, &win_attr);
    if (!win_attr.override_redirect && win_attr.map_state == IsViewable) {
        WeeDrawBorder(win);
        WeeCenterCursor(win);
        WeeRaiseAndFocus(win);
    }
}

void WeeRunCmd(char *cmd, char *env_var) {
    char system_cmd[512];

    if (env_var != NULL) {
        sprintf(system_cmd, "if [ \"%s\" ]; then sh -c \"%s &\"; else sh -c \"%s &\"; fi", env_var, env_var, cmd);
    }
    else {
        sprintf(system_cmd, "%s", cmd);
    }

    (void)(system(system_cmd)+1);
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
        WeeRunCmd("dmenu_run -l 5 -p '$(date +'%d %a %H:%M')'", "$WEEMA_LAUNCHER");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == t_key) {
        WeeRunCmd("x-terminal-emulator", "$WEEMA_TERMINAL");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == b_key) {
        WeeRunCmd("x-www-browser", "$WEEMA_BROWSER");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == l_key) {
        WeeRunCmd("slock", "$WEEMA_LOCK");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_up_key) {
        WeeRunCmd("amixer set Master 3+", "$WEEMA_VOLUMEUP");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == vol_down_key) {
        WeeRunCmd("amixer set Master 3+", "$WEEMA_VOLUMEDOWN");
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == print_key) {
        WeeRunCmd("scrot", "$WEEMA_PRINTSCREEN");
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
        WeeCenterCursor(ev.xcirculate.window);
        XSetInputFocus(display, ev.xcirculate.window, RevertToPointerRoot, CurrentTime);
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
    else if (ev.type == MapNotify) {
        WeeHandleNewWindow(ev.xmap.window);
    }
}

int main() {
    if (!(display = XOpenDisplay(0x0))) return 1;

    WeeInitRootWindow();
    WeeSetupGrab();

    WeeRunCmd("xsetroot -cursor_name arrow -solid \"#030609\"", NULL);
    WeeRunCmd("feh --bg-scale ~/wallpaper.jpg", "$WEEMA_INIT");
        
    for(;;) {
        WeeInterceptEvents();
    }
}
