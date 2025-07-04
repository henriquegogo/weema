// One-letter shortcuts will be binded 'Super-Letter'.
const char CMD_KEYS[][24][128] = {
  {"XF86AudioRaiseVolume", "amixer set Master 5%+"},
  {"XF86AudioLowerVolume", "amixer set Master 5%-"},
  {"equal", "amixer set Master 5%+"},
  {"minus", "amixer set Master 5%-"},
  {"Print", "scrot -s -e 'xclip -selection clipboard -t image/png -i $f'"},
  {"a", "dmenu_run"},
  {"t", "x-terminal-emulator &"},
  {"b", "x-www-browser &"},
  {"l", "slock"}
};

const char STARTUP_CMDS[][128] = {
  /*
  "xsetroot -solid '#223'",
  "xrandr --output HDMI-1-0 --auto --left-of eDP-1",
  "xcompmgr &"
  */
};
