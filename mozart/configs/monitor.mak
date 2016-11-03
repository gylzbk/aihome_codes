target := monitor
monitor_DIR := $(TOPDIR)/src/monitor

monitor_DEPENDENCIES := 

TARGETS += monitor
TARGETS1 += monitor

SUPPORT_BT_MODULE ?= 0
SUPPORT_VR ?= 0
SUPPORT_LAPSULE ?= 0
SUPPORT_DMR ?= 0
SUPPORT_WEBRTC ?= 0
SUPPORT_DMS ?= 0
SUPPORT_AIRPLAY ?= 0
SUPPORT_LOCALPLAYER ?= 0
SUPPORT_ATALK ?= 0
SUPPORT_LCD ?= 0
SUPPORT_SMARTUI ?= 0
SUPPORT_UPDATE_LEGACY ?= 0
SUPPORT_BOARD ?= 0
# do nothing
define monitor_CONFIGURE_CMDS


endef

define monitor_OPTS
	
endef

define monitor_BUILD_CMDS
	$(monitor_OPTS) \
			$(MAKE) -C $(monitor_DIR)
endef

define monitor_INSTALL_TARGET_CMDS
	$(monitor_OPTS) \
			$(MAKE) -C $(monitor_DIR) DESTDIR=$(MOZART_DIR) install
endef

define monitor_CLEAN_CMDS
	-$(monitor_OPTS) \
			$(MAKE) -C $(monitor_DIR) clean
endef

define monitor_UNINSTALL_TARGET_CMDS
	-$(monitor_OPTS) \
			$(MAKE1) -C $(monitor_DIR) DESTDIR=$(MOZART_DIR) uninstall
endef

$(eval $(call install_rules,monitor,$(monitor_DIR),target))
