BINDIR?=/usr/bin
PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall -Wno-maybe-uninitialized -Wno-unused-result

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include weema.c -L$(PREFIX)/lib -lX11 -o weema

install:
	install -d ${DESTDIR}${BINDIR}
	install -m 755 weema ${DESTDIR}${BINDIR}
		
clean:
	rm -f weema
