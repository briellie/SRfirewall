PREFIX=/usr/local

all: # Nothing here

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/lib/srfirewall \
	$(DESTDIR)$(PREFIX)/etc/srfirewall $(DESTDIR)$(PREFIX)/etc/srfirewall \
	$(DESTDIR)$(PREFIX)/share/doc/srfirewall
	cp -r bin/* $(DESTDIR)$(PREFIX)/bin/
	cp -r lib/* $(DESTDIR)$(PREFIX)/lib/srfirewall/
	cp -r etc/* $(DESTDIR)$(PREFIX)/etc/srfirewall/
	cp -r ChangeLog doc/* License ReadMe $(DESTDIR)$(PREFIX)/share/doc/srfirewall