target := mozart
mozart_DIR := $(TOPDIR)/src/mozart

TARGETS += mozart
TARGETS1 += mozart

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
# do nothing
define mozart_CONFIGURE_CMDS


endef

define mozart_OPTS
	BT=$(SUPPORT_BT_MODULE) VR=$(SUPPORT_VR) LAPSULE=$(SUPPORT_LAPSULE) WEBRTC=$(SUPPORT_WEBRTC) \
	DMR=$(SUPPORT_DMR) DMS=$(SUPPORT_DMS) AIRPLAY=$(SUPPORT_AIRPLAY) LOCALPLAYER=$(SUPPORT_LOCALPLAYER) \
	LCD=$(SUPPORT_LCD) ATALK=$(SUPPORT_ATALK) SMARTUI=$(SUPPORT_SMARTUI) UPDATE_LEGACY=$(SUPPORT_UPDATE_LEGACY)
endef

define mozart_BUILD_CMDS
	$(mozart_OPTS) \
			$(MAKE) -C $(mozart_DIR)
endef

define mozart_INSTALL_TARGET_CMDS
	$(mozart_OPTS) \
			$(MAKE) -C $(mozart_DIR) DESTDIR=$(MOZART_DIR) install
endef

define mozart_CLEAN_CMDS
	-$(mozart_OPTS) \
			$(MAKE) -C $(mozart_DIR) clean
endef

define mozart_UNINSTALL_TARGET_CMDS
	-$(mozart_OPTS) \
			$(MAKE1) -C $(mozart_DIR) DESTDIR=$(MOZART_DIR) uninstall
endef

$(eval $(call install_rules,mozart,$(mozart_DIR),target))
