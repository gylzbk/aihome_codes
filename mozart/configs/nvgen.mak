host-nvgen_DIR := $(TOPDIR)/tools/host-tools/nvgen

TARGETS += host-nvgen


# do nothing
define host-nvgen_CONFIGURE_CMDS


endef

define host-nvgen_BUILD_CMDS
	$(MAKE) -C $(host-nvgen_DIR)
endef

define host-nvgen_INSTALL_HOST_CMDS
	$(MAKE) -C $(host-nvgen_DIR) install DESTDIR=$(OUTPUT_DIR)/host
endef

define host-nvgen_CLEAN_CMDS
	-$(MAKE1) -C $(host-nvgen_DIR) clean DESTDIR=$(OUTPUT_DIR)/host
endef

define host-nvgen_DISTCLEAN_CMDS
	-$(MAKE1) -C $(host-nvgen_DIR) distclean DESTDIR=$(OUTPUT_DIR)/host
endef

define host-nvgen_UNINSTALL_HOST_CMDS
	-$(MAKE1) -C $(host-nvgen_DIR) uninstall DESTDIR=$(OUTPUT_DIR)/host
endef

$(eval $(call install_rules,host-nvgen,$(host-nvgen_DIR),host))
