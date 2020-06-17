BINDIR?=/usr/bin
XSESSIONSDIR?=/usr/share/xsessions
CFLAGS?=-Wpedantic -Wall -Wextra

all:
	$(CC) $(CFLAGS) weema.c -lX11 -o weema

install:
	install -d ${BINDIR}
	install -m 755 weema ${BINDIR}
	install -m 644 weema.desktop ${XSESSIONSDIR}
	update-alternatives --install /usr/bin/x-window-manager x-window-manager /usr/bin/weema 10

uninstall:
	update-alternatives --remove x-window-manager /usr/bin/weema
	rm ${BINDIR}/weema
	rm ${XSESSIONSDIR}/weema.desktop
		
clean:
	rm weema

dependencies:
	sudo apt install feh dmenu scrot alsa-utils
