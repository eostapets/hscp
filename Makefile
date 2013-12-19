# $Id: Makefile,v 1.1 2010/05/09 17:37:09 mizutani Exp mizutani $

CC=g++
LD=g++

all:
	$(MAKE) hscp

libudt.a:
	$(MAKE) -C udt4
	cp udt4/src/libudt.a .

libopenssh.a:
	$(MAKE) -C openssh
	cp openssh/libopenssh.a .

hscp: libudt.a libopenssh.a hscp.o udtscp.o
	$(LD) -o $@ hscp.o udtscp.o -L. -Wl,-Bstatic -ludt  -lopenssh -Wl,-Bdynamic -lstdc++ -lpthread

hscp.o: hscp.cpp hscp.h
	$(CC) -c -I./udt4/src -I./openssh hscp.cpp

udtscp.o: udtscp.cpp udtscp.h
	$(CC) -c -I./udt4/src udtscp.cpp

clean:
	rm -f *.o hscp udtscp.o libudt.a libopenssh.a
	$(MAKE) -C udt4 clean
	$(MAKE) -C openssh clean

install:
	mkdir -p $(DESTDIR)/usr/bin $(DESTDIR)/etc
	cp -p hscp $(DESTDIR)/usr/bin
	chown root:root $(DESTDIR)/usr/bin/hscp
	cp -p hscp.conf $(DESTDIR)/etc
	chown root:root $(DESTDIR)/etc/hscp.conf
	chmod 644 $(DESTDIR)/etc/hscp.conf

uninstall:
	rm -f $(DESTDIR)/usr/bin/hscp
	rm -f $(DESTDIR)/etc/hscp.conf
