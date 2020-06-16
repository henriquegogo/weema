/* Weema by Henrique Gogó <henriquegogo@gmail.com>, 2020.
 * MIT License */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

Display *display = NULL;
Window root_win;
XWindowAttributes root_attr, click_attr;
XButtonEvent click_start;

KeyCode up_key, down_key, left_key, right_key,
        r_key, t_key, l_key, b_key,
        vol_up_key, vol_down_key,
        f4_key, del_key, tab_key, print_key;

void InitRootWindow() {
    root_win = XDefaultRootWindow(display);
    XGetWindowAttributes(display, root_win, &root_attr);
    XSelectInput(display, root_win, SubstructureNotifyMask);
}

void GrabKey(int keycode, unsigned int modifiers) {
    XGrabKey(display, keycode, modifiers, root_win, True, GrabModeAsync, GrabModeAsync);
}

void GrabButton(int buttoncode, unsigned int modifiers) {
    XGrabButton(display, buttoncode, modifiers, root_win, True, ButtonPressMask,
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

void DrawBorder(Window win) {
    XSetWindowBorderWidth(display, win, 1);
    XSetWindowBorder(display, win, 0);
}

void MoveUp(Window win, XWindowAttributes win_attr) {
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

void MoveDown(Window win, XWindowAttributes win_attr) {
    int third_width = root_attr.width / 3;
    int third_height = root_attr.height / 3;
    int center_position = (root_attr.height - win_attr.height) / 2;
    Bool is_fullscreen = win_attr.width == root_attr.width && win_attr.height == root_attr.height;

    if (is_fullscreen) {
        XMoveResizeWindow(display, win, third_width / 2 + 1, third_height / 2,
                third_width * 2, third_height * 2);
    }
    else if (win_attr.y < center_position) {
        XMoveWindow(display, win, win_attr.x, center_position);
    }
    else {
        XMoveWindow(display, win, win_attr.x, root_attr.height - win_attr.height);
    }
}

void MoveLeft(Window win, XWindowAttributes win_attr) {
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

void MoveRight(Window win, XWindowAttributes win_attr) {
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

void ResizeUp(Window win, XWindowAttributes win_attr) {
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

void ResizeDown(Window win, XWindowAttributes win_attr) {
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

void ResizeLeft(Window win, XWindowAttributes win_attr) {
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

void ResizeRight(Window win, XWindowAttributes win_attr) {
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

void RaiseAndFocus(Window win) {
    XRaiseWindow(display, win);
    XSetInputFocus(display, win, RevertToPointerRoot, CurrentTime); 
}

void HandleClick(XButtonEvent button_event) {
    XGrabPointer(display, button_event.subwindow, True, PointerMotionMask|ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(display, button_event.subwindow, &click_attr);
    click_start = button_event;
}

void HandleMotion(XEvent ev) {
    while (XCheckTypedEvent(display, MotionNotify, &ev));

    int xdiff = ev.xbutton.x_root - click_start.x_root;
    int ydiff = ev.xbutton.y_root - click_start.y_root;

    if (click_start.button == 1) {
        XMoveWindow(display, ev.xmotion.window, click_attr.x + xdiff, click_attr.y + ydiff);
    }
    else if (click_start.button == 2) {
        XResizeWindow(display, ev.xmotion.window,
                abs(click_attr.width + xdiff) + 1, abs(click_attr.height + ydiff) + 1);
    }
}

void HandleNewWindow(Window win) {
    XWindowAttributes win_attr;
    XGetWindowAttributes(display, win, &win_attr);

    if (!win_attr.override_redirect && win_attr.map_state == IsViewable) {
        DrawBorder(win);
        RaiseAndFocus(win);
    }
}

Window GetCurrentWindow() {
    unsigned int i, nwins;
    Window win, *wins;
    XWindowAttributes win_attr;

    XQueryTree(display, root_win, &win, &win, &wins, &nwins);

    for (i = 0; i < nwins; i++) {
        XGetWindowAttributes(display, wins[i], &win_attr);

        if (wins[i] != None && !win_attr.override_redirect && win_attr.map_state == IsViewable) {
            win = wins[i];
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

    RaiseAndFocus(win);
}

void InterceptEvents() {
    XEvent ev;
    XNextEvent(display, &ev);

    if (ev.type == ButtonPress) {
        if (ev.xbutton.subwindow != None) {
            RaiseAndFocus(ev.xbutton.subwindow);
            HandleClick(ev.xbutton);
        }
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
        XCirculateSubwindowsDown(display, root_win);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == tab_key) {
        XCirculateSubwindowsUp(display, root_win);
    }
    else if (ev.type == CirculateNotify) {
        XSetInputFocus(display, GetCurrentWindow(), RevertToPointerRoot, CurrentTime); 
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == del_key) {
        XCloseDisplay(display);
    }
    else if (ev.type == KeyPress && ev.xkey.keycode == f4_key) {
        CloseWindow(GetCurrentWindow());
    }
    else if (ev.type == KeyPress) {
        HandleWindowPosition(GetCurrentWindow(), ev.xkey.keycode, ev.xkey.state);
    }
    else if (ev.type == MapNotify) {
        HandleNewWindow(ev.xmap.window);
    }
    else if (ev.type == DestroyNotify) {
        XSetInputFocus(display, GetCurrentWindow(), RevertToPointerRoot, CurrentTime); 
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
