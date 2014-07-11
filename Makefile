PREFIX=/usr/local
BINDIR=/bin

build:
	$(MAKE) -C src/

clean:
	$(MAKE) -C src/ clean

install: build
	install -d $(DESTDIR)$(PREFIX)$(BINDIR)
	install -m 0755 src/tdfs $(DESTDIR)$(PREFIX)$(BINDIR)
	#install -m 0644 man/tdfs.8 $(DESTDIR)$(PREFIX)/share/man/man8/ # Manual not updated
