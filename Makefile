BINDIR?=/usr/bin
XSESSIONSDIR?=/usr/share/xsessions
CFLAGS?=-Wpedantic -Wall -Wextra
DEPENDENCIES?=feh dmenu scrot alsa-utils
HIDEERRORS?=2> /dev/null || true

all:
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
