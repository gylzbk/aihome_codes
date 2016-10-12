DIR_CUR := $(shell pwd)

INSTALL := install
STRIP := mipsel-linux-strip
AR = mipsel-linux-ar
CC = mipsel-linux-gcc
CFLAGS := -g -fPIC -O2 -Werror -I$(MOLIB_DIR)/usr/include  -I$(MOLIB_UPDATER_DIR)/usr/include
CFLAGS += -I$(MOLIB_DIR)/usr/include -I$(MOLIB_DIR)/usr/include/mozart
CFLAGS += -I$(MOLIB_DIR)/include -I$(MOLIB_DIR)/include/curl -I$(MOLIB_DIR)/include/tingfm
LDFLAGS := -shared -lm -lpthread -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib \
	  -lini \
	  -lutils \
	  -lplayer \
	  -ltips \
	  -lasound \
	  -lrecord \
	  -lsharememory \
	  -lrt \
	  -ldl 
	  
#LDFLAGS += -laiserver -laicjson

#CONFIG = aec-m150
#CONFIG = aec-x1000
#CONFIG = aec-x1000-4mic
CONFIG = aec-x1000-1mic


DIR_LIB_VR := $(DIR_CUR)/libs
DIR_BIN_VR := $(DIR_CUR)/bin	
DIR_SRC_VR := $(DIR_CUR)/src
DIR_INCLUDE := $(DIR_CUR)/include

SRCS := ./src/ai_sound_dev.c ./src/ai_aec_local.c ./src/ai_cloud_sem.c ./src/ai_cloud_sem_text.c ./src/ai_cloud_syn.c 
SRCS += ./src/aiengine.c
RESOURCE = aihome
CFLAGS += -I$(DIR_SRC_VR)

GRAMMAR = #grammar
NATIVE_CMD = #native_cmd
WAKEUP_AEC = #wakeup_aec


LDFLAGS += -lecho_wakeup -laiengine -laiserver

ifeq ($(RESOURCE), aihome)
	CFLAGS += -DAIHOME
else
	CFLAGS += -DROBOT
endif

ifeq ($(WAKEUP_AEC), wakeup_aec)
	CFLAGS += -DWAKEUP_AEC
	SRCS += wakeup_aec.c
endif

ifeq ($(NATIVE_CMD), native_cmd)
	CFLAGS += -DNATIVE_CMD
endif

TARGET := libaispeech.so
OBJS = $(patsubst %.c, %.o, $(SRCS))

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $? -o $@ $(CFLAGS) $(LDFLAGS)

install: all
	$(INSTALL) -d $(DESTDIR)$(DIR_BIN)
	$(INSTALL) -d $(DESTDIR)$(DIR_INC)
	$(INSTALL) -d $(DESTDIR)$(DIR_LIB)
	$(INSTALL) -d $(DESTDIR)$(DIR_DATA)/bin
	
#	$(INSTALL) -m 664 $(DIR_INCLUDE)/*.h $(DESTDIR)$(DIR_INC)
	$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(DIR_CUR)/libaispeech.so $(MOLIB_DIR)/usr/lib/
#	$(INSTALL) -m 755 -s --strip-program=$(STRIP) $(DIR_CUR)/vr-speech_test $(DESTDIR)$(DIR_TEST)/
	$(INSTALL) -m 755 ./*.so  $(MOLIB_DIR)/usr/lib/
#	$(INSTALL) -m 664 ./bin/* $(MOLIB_DIR)/usr/share/vr/bin/
#	$(INSTALL) -m 664 ./include/*.h  $(MOLIB_DIR)/usr/include
clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(DESTDIR)/usr/lib/libaispeech.so
uninstall:
	if [ -e  $(MOLIB_DIR)/usr/lib/libaispeech.so ]; then \
		#rm -rf $(MOLIB_DIR)/usr/share/vr/bin/*.*  \
		rm -rf $(MOLIB_DIR)/usr/libaispeech.so; \
		fi

.PHONY: clean all install
