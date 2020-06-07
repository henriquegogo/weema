BINDIR?=/usr/bin
XSESSIONSDIR?=/usr/share/xsessions
CFLAGS?=-Wpedantic -Wall -Wextra

all:
	$(CC) $(CFLAGS) weema.c -lX11 -o weema

debug:
	$(CC) $(CFLAGS) -g weema.c -lX11 -o weema-debug
	gdb ./weema-debug

install:
	install -d ${BINDIR}
	install -m 755 weema-cmd ${BINDIR}
	install -m 755 weema ${BINDIR}
	install -m 644 weema.desktop ${XSESSIONSDIR}

uninstall:
	rm ${BINDIR}/weema-cmd
	rm ${BINDIR}/weema
	rm ${XSESSIONSDIR}/weema.desktop
		
clean:
	rm weema
	rm weema-debug
