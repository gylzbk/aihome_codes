updater_DIR := $(TOPDIR)/src/updater

TARGETS += updater
TARGETS1 += updater

SUPPORT_SMARTUI ?= 0

define updater_OPTS
	SMARTUI=$(SUPPORT_SMARTUI)
endef

define updater_BUILD_CMDS
	$(updater_OPTS) \
		$(MAKE) -C $(@D)
endef

define updater_INSTALL_TARGET_CMDS
	$(updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) install
endef

define updater_CLEAN_CMDS
	-$(updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) clean
endef

define updater_UNINSTALL_TARGET_CMDS
	-$(updater_OPTS) \
		$(MAKE) -C $(@D) DESTDIR=$(MOZART_UPDATER_DIR) uninstall
endef

ifeq ($(SUPPORT_UPDATE_LEGACY), 1)
$(eval $(call install_rules,updater,$(updater_DIR),target))
endif
