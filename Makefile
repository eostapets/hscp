# $Id: Makefile,v 1.1 2010/05/09 17:37:09 mizutani Exp mizutani $

#SSHSRCDIR=../openssh-5.4p1
#SSHSRCDIR=../openssh-5.5p1
#SSHSRCDIR=../openssh-5.6p1
SSHSRCDIR=../openssh-5.8p1

CC=g++
LD=g++
CPPFLAGS=-I$(SSHSRCDIR) -I. -I$(UDTSRCDIR)

all:
	$(MAKE) hscp

libudt.a:
	$(MAKE) -C udt4
	cp udt4/src/libudt.a .

hscp: libudt.a hscp.o udtscp.o
#	$(LD) -o $@ hscp.o udtscp.o $(UDTSRCDIR)/libudt.a $(SSHSRCDIR)/progressmeter.o $(SSHSRCDIR)/bufaux.o -L$(SSHSRCDIR) -lssh -L$(SSHSRCDIR)/openbsd-compat -lopenbsd-compat -L$(UDTSRCDIR) -ludt -lstdc++ -lpthread
	$(LD) -o $@ hscp.o udtscp.o $(UDTSRCDIR)/libudt.a $(SSHSRCDIR)/bufaux.o -L$(SSHSRCDIR) -lssh -L$(SSHSRCDIR)/openbsd-compat -lopenbsd-compat -L$(UDTSRCDIR) -ludt -lstdc++ -lpthread

udtscp.o: udtscp.cpp udtscp.h
	$(CC) -g -c -I$(UDTSRCDIR) udtscp.cpp

clean:
	rm -f *.o hscp udtscp.o libudt.a
	$(MAKE) -C udt4 clean

install_bin:
	cp -p hscp /usr/local/bin
#	chown root:root /usr/local/bin/hscp
#	chmod 755 /usr/local/bin/hscp
#	cp -p hscp.conf /etc
#	chown root:root /etc/hscp.conf
#	chmod 644 /etc/hscp.conf

install:
	cp -p hscp /usr/local/bin
	chown root:root /usr/local/bin/hscp
	chmod 755 /usr/local/bin/hscp
	cp -p hscp.conf /usr/local/etc
	chown root:root /usr/local/etc/hscp.conf
	chmod 644 /usr/local/etc/hscp.conf

uninstall:
	rm -f /usr/local/bin/hscp
	rm -f /usr/local/etc/hscp.conf
