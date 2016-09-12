target := libaispeech
libaispeech_DIR := $(TOPDIR)/src/aispeech/speech

TARGETS += libaispeech
TARGETS1 += libaispeech


# do nothing
define libaispeech_CONFIGURE_CMDS


endef

define libaispeech_BUILD_CMDS
	$(MAKE) -C $(libaispeech_DIR)
endef

define libaispeech_INSTALL_TARGET_CMDS
	$(MAKE) -C $(libaispeech_DIR) DESTDIR=$(MOLIB_DIR) install
endef

define libaispeech_CLEAN_CMDS
	-$(MAKE1) -C $(libaispeech_DIR) clean
endef

# do nothing
define libaispeech_DISTCLEAN_CMDS


endef

define libaispeech_UNINSTALL_CMDS
	-$(MAKE1) -C $(libaispeech_DIR) DESTDIR=$(MOLIB_DIR) uninstall
endef

$(eval $(call install_rules,libaispeech,$(libaispeech_DIR),target))
