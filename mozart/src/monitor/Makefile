DIR_CUR := $(shell pwd)

# $1: string
# $2: start pos
# $3: length
define substring
$(shell echo `expr substr $(1) $(2) $(3)`)
endef

BT ?= 0
VR ?= 0
LAPSULE ?= 0
DMR ?= 0
WEBRTC ?= 0
DMS ?= 0
AIRPLAY ?= 0
ATALK ?= 0
AITALK ?= 0
LOCALPLAYER ?= 0

CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
STRIP = mipsel-linux-strip
INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin

SRCS = monitor.c

# common opts.
CFLAGS = -fPIC -Wall -g -O2 -I. -I$(MOLIB_DIR)/usr/include  -I$(MOLIB_DIR)/usr/include/mozart -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include -I$(DIR_CUR)/include/
CFLAGS += -rdynamic -Wall -Werror
CFLAGS += -DMOZART_RELEASE_NAME

LDFLAGS = -g -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib


OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET = monitor

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:
	$(INSTALL) -d $(DESTDIR)$(SBINDIR)
	$(INSTALL) -d $(DESTDIR)$(INITDIR)
	if [ -e $(TARGET) ]; then \
		$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(TARGET) $(DESTDIR)$(SBINDIR)/; \
	fi

clean:
	-rm -rf $(OBJS) $(TARGET)

uninstall:
	if [ -e $(DESTDIR)$(SBINDIR)/$(TARGET)]; then \
		rm -rf $(DESTDIR)$(SBINDIR)/$(TARGET); \
	fi

.PHONY:all clean install uninstall $(TARGET)
