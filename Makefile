BINDIR?=/usr/bin
PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall -Wextra

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include weema.c -L$(PREFIX)/lib -lX11 -o weema

install:
	install -d ${BINDIR}
	install -m 755 weema-cmd ${BINDIR}
	install -m 755 weema ${BINDIR}

uninstall:
	rm ${BINDIR} weema-cmd
	rm ${BINDIR} weema
		
clean:
	rm -f weema
