# NV Image generator Makefile

CC = gcc
INSTALL = install

DESTDIR =
PREFIX = /usr
LIBDIR = $(PREFIX)/lib
SBINDIR = $(PREFIX)/sbin
INCDIR = $(PREFIX)/include
TESTDIR = $(PREFIX)/test

SRCS = nvgen.c
HEADER = nv_wr.h

CFLAGS = -m32 -Wall -Werror

all: nvgen

nvgen: $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

install: nvgen
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 755 nvgen $(DESTDIR)$(SBINDIR)

uninstall:
	-rm -rf $(DESTDIR)$(SBINDIR)/nvgen

clean:
	@-rm nvgen

distclean: clean

.PHONY: all nvgen install uninstall clean distclean
