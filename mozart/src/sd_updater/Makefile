LCD ?= 0
CC = mipsel-linux-gcc
STRIP = mipsel-linux-strip
INSTALL = install
DESTDIR =
PREFIX = /usr
SBINDIR = $(PREFIX)/sbin
INITDIR = /etc/init.d/

SRCS = update.c nv_rw.c

CFLAGS = -Wall -O2 -I. -I$(MOLIB_DIR)/usr/include -I$(MOZART_DIR)/usr/include
CFLAGS += -I$(MOLIB_DIR)/usr/include/upnp
CFLAGS += -I$(MOLIB_UPDATER_DIR)/usr/include

LDFLAGS = -lpthread -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LDFLAGS += -lini -lutils

ifeq ($(SMARTUI),1)
    CFLAGS += -DSUPPORT_SMARTUI
    LDFLAGS += -llinklist -lsmartui
else
    $(warning lcd func CLOSED!!!!)
endif

OBJS = $(patsubst %.c, %.c.o, $(SRCS))

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

all:update

update:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

install:
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)
	$(INSTALL) -m 755 -s --strip-program=$(STRIP) update $(DESTDIR)$(SBINDIR)/
	$(INSTALL) -m 755 -s --strip-program=$(STRIP) flash_erase $(DESTDIR)$(SBINDIR)/
	$(INSTALL) -m 755 S99updater.sh $(DESTDIR)$(INITDIR)/

clean:
	-rm -rf *.o update

uninstall:

.PHONY:all clean install uninstall $(TARGETS)
