CC = mipsel-linux-gcc
CXX= mipsel-linux-g++
STRIP = mipsel-linux-strip

CFLAGS := -fPIC -Werror -O2 
CFLAGS += -I$(MOLIB_DIR)/include -I$(MOLIB_DIR)/include/curl -I$(MOLIB_DIR)/usr/include -I$(MOLIB_DIR)/usr/include/curl -I/opt/aispeech/doss/doss_codes/mozart/src/aispeech/server/src
CFLAGS += -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOLIB_UPDATER_DIR)/usr/include/alsa 
LDFLAGS = -shared -lpthread -lm -lcurl -L$(MOLIB_DIR)/usr/lib -L./libs -lzlog -lcloud_param -lelife_doss 

DESTDIR =
PREFIX = /usr
BINDIR = $(PREFIX)/bin
SBINDIR = $(PREFIX)/sbin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

INSTALL    = install

#SRCS := $(wildcard ./src/*.c)
SRCS := ./src/ai_error.c  ./src/ai_music_list.c ./src/ai_server.c  ./src/ai_slot.c ./src/ai_song_recommend.c  
SRCS += ./src/ai_zlog.c
SRCS += ./src/ai_aitalk_send.c ./src/list.c ./src/music_list.c ./src/iota.c ./src/cJSON.c ./src/fop.c ./src/json_op.c 

OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET=libaiserver.so

all:$(TARGET)

libaiserver.so: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.c.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

install:$(TARGET)
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(INCDIR)
#	$(INSTALL) -m 664  src/*.h $(DESTDIR)$(INCDIR)
	#$(INSTALL) -m 664  ../include/*.h $(MOLIB_DIR)/usr/include/
	#$(INSTALL) -m 664  ../include/tingfm/*.h $(MOLIB_DIR)/usr/include/tingfm/
	$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(TARGET) $(MOLIB_DIR)/usr/lib/

clean:
	-rm -f $(OBJS) $(TARGET)

uninstall:
	-rm -f $(MOLIB_DIR)/usr/lib/libaiserver.so
	#-rm -f $(MOLIB_DIR)/usr/include/ai_server.h
	#-rm -f $(MOLIB_DIR)/usr/include/ai_slot_type.h
.PHONY:all clean install uninstall $(TARGET)
