define ROOTFS_cpio_CMDS
cd $(1) && find . | cpio --quiet -oH  newc > $(2)
endef

define ROOTFS_tarball_CMDS
cd $(1) && tar cf $(2) .
endef

ROOTFS_ext4_DEPENDENCIES := host-genext2fs host-e2fsprogs
define ROOTFS_ext4_CMDS
GEN=4 REV=1 $(TOPDIR)/configs/genext2fs.sh -d $(1) $(2)
endef

ROOTFS_cramfs_DEPENDENCIES := host-mkfs.cramfs
define ROOTFS_cramfs_CMDS
$(OUTPUT_DIR)/host/usr/sbin/mkfs.cramfs $(1) $(2)
endef

ifeq ("$(SUPPORT_FS)", "ramdisk")
$(eval $(call install_fs_rules,cpio,1))
endif
ifeq ("$(SUPPORT_FS)", "ext4")
TARGETS += $(ROOTFS_ext4_DEPENDENCIES)
$(eval $(call install_fs_rules,ext4,0))
endif
ifeq ("$(SUPPORT_FS)", "cramfs")
TARGETS += $(ROOTFS_cramfs_DEPENDENCIES)
$(eval $(call install_fs_rules,cramfs,0))
ifneq ($(SUPPORT_UPDATE_LEGACY), 1)
$(eval $(call install_fs_rules,cpio,1))
endif
endif
#$(eval $(call install_fs_rules,tarball,0))

##################################################################
USRDATA_jffs2_DEPENDENCIES := host-mkfs.jffs2
define USRDATA_jffs2_CMDS
$(OUTPUT_DIR)/host/usr/sbin/mkfs.jffs2 -m none -r $(1) -s$(USRDATA_FLASH_PAGESIZE) \
	-e$(USRDATA_FLASH_ERASESIZE) -p$(USRDATA_FLASH_PADSIZE) -o $(2)
endef

ifeq ("$(SUPPORT_USRDATA)", "jffs2")
TARGETS += $(USRDATA_jffs2_DEPENDENCIES)
$(eval $(call install_usrdata_rules,jffs2,0))
endif

##################################################################
NV_IMAGE_DEPENDENCIES := host-nvgen
NV_IMAGE_CMD := $(OUTPUT_DIR)/host/usr/sbin/nvgen -b 1024 -c 128

TARGETS += $(NV_IMAGE_DEPENDENCIES)
ifneq ($(SUPPORT_UPDATE_LEGACY), 1)
NV_IMAGE_CMD += -e $(MOZART_VERSION)
else
NV_IMAGE_CMD += -i $(MOZART_VERSION_LEGACY)
endif

ifeq ($(SUPPORT_BATTERY_NV_FLAG), 1)
NV_IMAGE_CMD += -x
endif

define NV_IMAGE_CMDS
	$(NV_IMAGE_CMD) -f $(1)
endef

$(eval $(call install_nvimage_rules))

##################################################################
define UPDATE_PACKAGE_CMD
mkdir -p $(OUTPUT_DIR)/image
@cp $(TARGET_DIR)/*.$(SUPPORT_FS) $(TARGET_DIR)/*.jffs2 $(OUTPUT_DIR)/image
$(TOPDIR)/tools/host-tools/update_pack/gen_tool/gen_otaupdate.sh v$(1) $(TARGET_DIR)/updatepkg $(OUTPUT_DIR)/image
@rm -rf $(OUTPUT_DIR)/image
endef
