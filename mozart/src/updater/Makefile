CC = mipsel-linux-gcc
CXX= mipsel-linux-g++
AR = mipsel-linux-ar
STRIP = mipsel-linux-strip

CFLAGS= -g -fPIC -Wall -Werror -O2 -I. -I$(MOLIB_UPDATER_DIR)/usr/include -Ilibflash -Ilibupdater -Icommon
CFLAGS += -rdynamic -fasynchronous-unwind-tables
LDFLAGS= -L$(MOLIB_UPDATER_DIR)/usr/lib -Llibflash -Llibupdater -lupdater -lnvrw -lwifi -ltips -lplayer -lsharememory
LDFLAGS += -Wl,-rpath-link,$(MOLIB_UPDATER_DIR)/usr/lib

DESTDIR ?=
PREFIX = /usr
BINDIR = $(PREFIX)/bin
ETCDIR = /etc
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

INSTALL    = install

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.c.o, $(SRCS))

updater_SRCS = $(wildcard libupdater/*.c)
updater_OBJS = $(patsubst %.c, %.c.o, $(updater_SRCS))
updater_CFLAGS = -g -Wall -Werror -O2 -I$(MOLIB_UPDATER_DIR)/usr/include -Ilibflash -Ilibupdater -Icommon -fPIC
updater_LDFLAGS = -shared -L$(MOLIB_UPDATER_DIR)/usr/lib -Llibflash -lini -lcrypto -lssl -lflash -lutils -lnvrw

flash_SRCS = $(wildcard libflash/*.c)
flash_OBJS = $(patsubst %.c, %.c.o, $(flash_SRCS))
flash_CFLAGS = -g -Wall -Werror -O2 -I$(MOLIB_UPDATER_DIR)/usr/include -Ilibflash -Icommon -fPIC

TARGETS = libupdater/libupdater.so libflash/libflash.a updater
all:$(TARGETS)


updater: libupdater/libupdater.so $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

libupdater/%.c.o:libupdater/%.c
	$(CC) $(updater_CFLAGS) -c $^ -o $@

libflash/%.c.o:libflash/%.c
	$(CC) $(flash_CFLAGS) -c $^ -o $@

libflash/libflash.a:$(flash_OBJS)
	$(AR) rs $@ $^

libupdater/libupdater.so: libflash/libflash.a $(updater_OBJS)
	$(CC) -o $@ $(updater_OBJS) $(updater_LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -f $(OBJS) $(flash_OBJS) $(updater_OBJS) $(TARGETS)

install:
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -d $(DESTDIR)$(INCDIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)
	$(INSTALL) -m 755 libupdater/libupdater.so $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 755 libupdater/updater_interface.h $(DESTDIR)$(INCDIR)
	$(INSTALL) -m 755 updater $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 755 S99updater.sh $(DESTDIR)$(INITDIR)/

uninstall:
	rm -rf $(DESTDIR)$(LIBDIR)/libupdater/libupdater.so
	rm -rf $(DESTDIR)$(INCDIR)/libupdater/updater_interface.h
	rm -rf $(DESTDIR)$(SBINDIR)/updater

.PHONY:all clean install uninstall $(TARGETS) libflash/libflash.a
