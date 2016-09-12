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
LOCALPLAYER ?= 0

CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
STRIP = mipsel-linux-strip
INSTALL = install
DESTDIR =
PREFIX = /usr
INITDIR = /etc/init.d/
SBINDIR = $(PREFIX)/sbin

SRCS = mozart.c mozart_module.c mozart_app.c mozart_event.c mozart_player.c
SRCS += modules/mozart_prompt_tone.c modules/mozart_net.c modules/mozart_linein.c modules/mozart_dmr.c
SRCS += mozart_battery.c mozart_misc.c

# common opts.
CFLAGS = -fPIC -Wall -g -O2 -I. -I$(MOLIB_DIR)/usr/include  -I$(MOLIB_DIR)/usr/include/mozart -I$(MOLIB_UPDATER_DIR)/usr/include -I$(MOZART_DIR)/usr/include -I$(DIR_CUR)/include/
CFLAGS += -rdynamic -fasynchronous-unwind-tables -Wall -Werror
#CFLAGS += -DFIXED_VOLUME
#CFLAGS += -DMOZART_RELEASE
CFLAGS += -DMOZART_RELEASE_NAME

LDFLAGS = -g -lpthread -lstdc++ -L$(MOLIB_DIR)/usr/lib -L$(MOLIB_UPDATER_DIR)/usr/lib -L$(MOZART_DIR)/usr/lib
LDFLAGS += -Wl,-rpath-link,$(MOLIB_DIR)/usr/lib:$(MOLIB_UPDATER_DIR)/usr/lib:$(MOZART_DIR)/usr/lib

# basic depends.
LDFLAGS += -levent -lpower -lwifi -lvolume -ltips -lsharememory -lini -lutils -lpthread -lplayer -lresample  
LDFLAGS += -lwifi -ltips -lutils -lplayer -lvolume -ljson-c -llinklist -lcurl -lssl -lcrypto -lz -lrt 


ifeq ("$(VR)","0")
$(warning vr func are CLOSED!!!!)
else
  ifeq ("$(VR)","vr_speech")
    LDFLAGS +=  -laiserver -laispeech  #-lvr_atalk
 #   SRCS += modules/aitalk/mozart_aispeech.c
	SRCS += modules/aitalk/mozart_aitalk_asr.c
	SRCS += modules/aitalk/mozart_aitalk_cloudplayer_control.c  modules/aitalk/mozart_aitalk_cloudplayer.c 
	SRCS += modules/aitalk/mozart_aitalk.c modules/aitalk/mozart_aitalk_localplayer.c 
  else ifeq ("$(VR)","vr_atalk")
    LDFLAGS += -lvr_atalk	
   	SRCS += modules/atalk/mozart_atalk_asr.c 
  endif
endif

ifeq ("$(BOARD)","0")
$(warning vr func are CLOSED!!!!)
else
  ifeq ("$(BOARD)","board_ds1825")
    SRCS += mozart_smartui.c
  else ifeq ("$(BOARD)","board_wb38")
    SRCS += mozart_smartui_wb38.c
  endif
endif

ifeq ($(SMARTUI),1)
LDFLAGS += -lsmartui
CFLAGS += -DSUPPORT_SMARTUI
else
$(warning lcd func CLOSED!!!!)
endif

ifeq ($(ATALK),1) # atalk
   	SRCS += modules/atalk/mozart_atalk_cloudplayer_control.c
	SRCS += modules/atalk/mozart_atalk.c modules/atalk/mozart_atalk_localplayer.c
	SRCS += modules/atalk/mozart_atalk_cloudplayer_control.c modules/atalk/dl_perform.c  modules/atalk/mozart_atalk_cloudplayer.c
else
$(warning atalk func CLOSED!!!!)
endif

ifeq ($(AITALK),1) # aispeech
<<<<<<< HEAD
	
=======
	SRCS += modules/aitalk/mozart_aitalk_cloudplayer_control.c  modules/aitalk/mozart_aitalk_cloudplayer.c 
	SRCS += modules/aitalk/mozart_aitalk.c modules/aitalk/mozart_aitalk_localplayer.c
>>>>>>> eaa8fab76949d2f1425a7024fdd0270b59f91b14
else
$(warning aitalk func CLOSED!!!!)
endif

ifeq ($(WEBRTC),1)
LDFLAGS += -lwebrtc_audio_processing -lwebrtc
else
$(warning webrtc func CLOSED!!!!)
endif


ifeq ($(DMR),1)
CFLAGS += -I$(MOLIB_DIR)/usr/include/upnp
LDFLAGS += -lrender
else
$(warning dmr func CLOSED!!!!)
endif

ifeq ($(AIRPLAY),1)
SRCS += modules/mozart_airplay.c
else
$(warning airplay func CLOSED!!!!)
endif

ifeq ($(LOCALPLAYER),1)
LDFLAGS += -llocalplayer
else
$(warning localplayer func CLOSED!!!!)
endif

ifeq ($(BT),0)
$(warning bt func are CLOSED!!!!)
else
  ifeq ("$(call substring,$(BT),1,3)", "rtk") # realtek's bt module.
    SRCS += bt.c
  else ifeq ("$(call substring,$(BT),1,3)", "bcm") # broadcom's bt module.
    SRCS += modules/mozart_bt_avk.c modules/mozart_bt_avk_fft.c modules/mozart_bt_hs.c modules/mozart_bt_hs_ring.c
    LDFLAGS += -lbsa -lbt -lm -lappcommon
  else
    $(error "$(BT): NOT supported bt module!!!!")
  endif
endif

ifneq ($(UPDATE_LEGACY),1)
    SRCS += mozart_update.c mozart_nvrw.c
    SRCS += modules/mozart_update_control.c
else
    $(info update legacy deployed!!!)
    SRCS += mozart_update_legacy.c
    SRCS += modules/mozart_update_control_legacy.c
    LDFLAGS += -lota
endif


OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET = mozart

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
		$(INSTALL) -m 755 S10mozart.sh $(DESTDIR)$(INITDIR)/; \
	fi

clean:
	-rm -rf $(OBJS) $(TARGET)

uninstall:
	if [ -e $(DESTDIR)$(SBINDIR)/$(TARGET)]; then \
		rm -rf $(DESTDIR)$(SBINDIR)/$(TARGET); \
		rm -rf $(DESTDIR)$(INITDIR)/S10mozart.sh; \
	fi

.PHONY:all clean install uninstall $(TARGET)
