host-mkfs.jffs2_DIR := $(TOPDIR)/tools/host-tools/mtd-utils-1.5.1

define host-mkfs.jffs2_BUILD_CMDS
        $(MAKE) -C $(host-mkfs.jffs2_DIR) WITHOUT_XATTR=1 mkfs.jffs2
endef

define host-mkfs.jffs2_INSTALL_HOST_CMDS
        $(MAKE) -C $(host-mkfs.jffs2_DIR) WITHOUT_XATTR=1 DESTDIR=$(OUTPUT_DIR)/host install-sbinJFFS2
endef

define host-mkfs.jffs2_DISTCLEAN_CMDS
        -$(MAKE1) -C $(host-mkfs.jffs2_DIR) clean
endef

$(eval $(call install_rules,host-mkfs.jffs2,$(host-mkfs.jffs2_DIR),host))
