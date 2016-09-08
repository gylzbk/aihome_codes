sd_updater_DIR := $(TOPDIR)/src/sd_updater

TARGETS += sd_updater
TARGETS1 += sd_updater

SUPPORT_SMARTUI ?= 0

define sd_updater_OPTS
	SMARTUI=$(SUPPORT_SMARTUI)
endef

define sd_updater_BUILD_CMDS
	$(sd_updater_OPTS) \
		$(MAKE) -C $(@D)
endef

define sd_updater_INSTALL_TARGET_CMDS
	$(sd_updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) install
endef

define sd_updater_CLEAN_CMDS
	-$(sd_updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) clean
endef

define sd_updater_UNINSTALL_TARGET_CMDS
	-$(sd_updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) uninstall
endef

ifneq ($(SUPPORT_UPDATE_LEGACY), 1)
$(eval $(call install_rules,sd_updater,$(sd_updater_DIR),target))
endif
