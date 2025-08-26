BINDIR?=/usr/bin
XSESSIONSDIR?=/usr/share/xsessions
CFLAGS?=-Wpedantic -Wall -Wextra
DEPENDENCIES?=alsa-utils brightnessctl rofi scrot suckless-tools x11-xserver-utils xbindkeys xcompmgr xdotool
HIDEERRORS?=2> /dev/null || true

all:
	cp --update=none .xbindkeysrc $(HOME)/.xbindkeysrc
	cp --update=none .xprofile $(HOME)/.xprofile
	$(CC) $(CFLAGS) weema.c -lX11 -o weema

install:
	install -d ${BINDIR}
	install -m 755 weema ${BINDIR}
	install -m 644 weema.desktop ${XSESSIONSDIR}

uninstall:
	rm ${BINDIR}/weema
	rm ${XSESSIONSDIR}/weema.desktop
		
clean:
	rm weema

dependencies:
	-@sudo apt install ${DEPENDENCIES} ${HIDEERRORS}
	-@sudo yum install ${DEPENDENCIES} ${HIDEERRORS}
	-@sudo pacman -S install ${DEPENDENCIES} ${HIDEERRORS}
