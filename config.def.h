// One-letter shortcuts will be binded 'Super-Letter'.
const char CMD_KEYS[][24][32] = {
  {"XF86AudioRaiseVolume", "amixer set Master 3%+"},
  {"XF86AudioLowerVolume", "amixer set Master 3%-"},
  {"Print", "scrot -s"},
  {"r", "dmenu_run"},
  {"t", "x-terminal-emulator &"},
  {"b", "x-www-browser &"},
  {"l", "slock"}
};
