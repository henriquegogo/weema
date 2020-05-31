if [ $1 = "launcher" ]; then
  dmenu_run
elif [ $1 = "volumeup" ]; then
  amixer set Master 3+
elif [ $1 = "volumedown" ]; then
  amixer set Master 3-
elif [ $1 = "printscreenarea" ]; then
  scrot -s
fi
