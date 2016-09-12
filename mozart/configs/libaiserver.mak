libaiserver_DIR := $(TOPDIR)/src/aispeech/server
libaiserver_DEPENDENCIES := hostapd

TARGETS += libaiserver

define libaiserver_BUILD_CMDS
	$(MAKE) -C $(@D)
endef

define libaiserver_INSTALL_TARGET_CMDS
	$(MAKE) -C $(@D) DESTDIR=$(MOZART_DIR) install
endef

define libaiserver_CLEAN_CMDS
	-$(MAKE) -C $(@D) clean
endef

define libaiserver_UNINSTALL_TARGET_CMDS
	-$(MAKE) -C $(@D) DESTDIR=$(MOZART_DIR) uninstall
endef

$(eval $(call install_rules,libaiserver,$(libaiserver_DIR),target))
