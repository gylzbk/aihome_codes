CC = gcc
CFLAGS = -m32 -Wall -I. -I$(MOLIB_UPDATER_DIR)/usr/include
LDFLAGS = -m32 -lpthread -L$(MOLIB_UPDATER_DIR)/usr/lib -lini_x86

DESTDIR =
PREFIX = /usr
LIBDIR = $(PREFIX)/lib
BINDIR = $(PREFIX)/bin
INCDIR = $(PREFIX)/include
TESTDIR = $(PREFIX)/test

INSTALL    = install

SRCS = nvgen.c
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

all:nvgen nvparser

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

nvgen: nvgen.c.o
	$(CC) $? $(LDFLAGS) -o $@

nvparser: nvparser.c.o
	$(CC) $? $(LDFLAGS) -o $@

install:$(TARGETS)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 755 nvgen $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 755 nvparser $(DESTDIR)$(BINDIR)

clean:
	-rm -rf nvgen nvparser *.o

distclean: clean

uninstall:
	rm -rf $(DESTDIR)$(BINDIR)/nvgen
	rm -rf $(DESTDIR)$(BINDIR)/nvparser

.PHONY:all clean install uninstall nvgen nvparser
