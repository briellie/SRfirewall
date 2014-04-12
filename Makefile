PREFIX=/usr/local

all: # Nothing here

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/lib/srfirewall \
	$(DESTDIR)$(PREFIX)/etc/srfirewall
	cp -r bin/* $(DESTDIR)$(PREFIX)/bin/
	cp -r lib/* $(DESTDIR)$(PREFIX)/lib/srfirewall/
	if [ ! -e $(DESTDIR)$(PREFIX)/etc/srfirewall/main.conf ]; then cp -r etc/* $(DESTDIR)$(PREFIX)/etc/srfirewall/; fi
	
install-etc:
	cp -r etc/* $(DESTDIR)$(PREFIX)/etc/srfirewall/
	
install-docs:
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/srfirewall
	cp -r ChangeLog doc/* License ReadMe $(DESTDIR)$(PREFIX)/share/doc/srfirewall
	
upgrade:
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/lib/srfirewall \
	$(DESTDIR)$(PREFIX)/etc/srfirewall
	cp -r bin/* $(DESTDIR)$(PREFIX)/bin/
	cp -r lib/* $(DESTDIR)$(PREFIX)/lib/srfirewall/