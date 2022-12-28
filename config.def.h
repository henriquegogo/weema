// One-letter shortcuts will be binded 'Super-Letter'.
const char CMD_KEYS[][24][128] = {
  {"XF86AudioRaiseVolume", "amixer set Master 5%+"},
  {"XF86AudioLowerVolume", "amixer set Master 5%-"},
  {"Print", "scrot -s"},
  {"r", "dmenu_run"},
  {"t", "x-terminal-emulator &"},
  {"b", "x-www-browser &"},
  {"l", "slock"},
  {"1", "xdotool search --onlyvisible --name \"$(xdotool search --onlyvisible --name \".{1,}\" getwindowname %@ | dmenu -l 5)\" windowfocus"}
};
