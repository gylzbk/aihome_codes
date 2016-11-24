# This top-level Makefile can *not* be executed in parallel
.NOTPARALLEL:

# absolute path
TOPDIR := $(shell pwd)

# we want bash as shell
SHELL:=$(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi; fi)

export SHELL

HOSTCC:=$(shell which gcc || echo gcc)
HOSTCXX:=$(shell which g++ || echo g++)
HOSTAR:=$(shell which ar || echo ar)
HOSTAS:=$(shell which as || echo as)
HOSTLD:=$(shell which ld || echo ld)
HOSTLN:=$(shell which ln || echo ln)
HOSTMAKE:=$(shell which make || echo make)
HOSTCMAKE:=$(shell which make || echo make)


# scale the maximum concurrency with the number of CPUs.
# An additional job is used in order to keep processors busy
# If the number of processors is not available, assume one.
PARALLEL_JOBS:=$(shell echo $$((1 + `getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1`)))

MAKE1:=$(HOSTMAKE) -j1
MAKE:=$(HOSTMAKE) -j$(PARALLEL_JOBS)
CMAKE:=cmake

export HOSTAR HOSTAS HOSTCC HOSTCXX HOSTLD HOSTMAKE HOSTCMAKE MAKE MAKE1 CMAKE

# Make sure pkg-config doesn't look outside the buildroot tree
unexport PKG_CONFIG_PATH
unexport PKG_CONFIG_SYSROOT_DIR

# Having DESTDIR set in the environment confuses the installation
# steps of some packages.
unexport DESTDIR


# Hide troublesome environment variables from sub processes
unexport CROSS_COMPILE
unexport ARCH
unexport CC
unexport CXX
unexport CPP
unexport CFLAGS
unexport CXXFLAGS
unexport LDLAGS
unexport GREP_OPTIONS
unexport CONFIG_SITE

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE=$(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE=0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet=
  Q=
ifndef VERBOSE
  VERBOSE=1
endif
else
  quiet=quiet_
  Q=@
endif

-include configs/version.mk
export MOZART_VERSION

OUTPUT_DIR := $(TOPDIR)/output

MOZART_DIR := $(OUTPUT_DIR)/mozart/app
MOZART_APP_DIR := $(MOZART_DIR)
MOZART_UPDATER_DIR := $(OUTPUT_DIR)/mozart/updater

MOLIB_DIR := $(OUTPUT_DIR)/molib/app
MOLIB_APP_DIR:= $(MOLIB_DIR)
MOLIB_UPDATER_DIR := $(OUTPUT_DIR)/molib/updater

TARGET_DIR := $(OUTPUT_DIR)/target
UPDATEPKG_DIR := $(OUTPUT_DIR)/updatepkg

APPFS_DIR := $(OUTPUT_DIR)/fs/appfs
UPDATERFS_DIR := $(OUTPUT_DIR)/fs/updaterfs
USRDATAFS_DIR := $(OUTPUT_DIR)/fs/usrdata

export TOPDIR
export OUTPUT_DIR
export MOZART_DIR
export MOZART_APP_DIR
export MOZART_UPDATER_DIR
export MOLIB_DIR
export MOLIB_APP_DIR
export MOLIB_UPDATER_DIR
export TARGET_DIR
export UPDATEPKG_DIR
export APPFS_DIR
export USRDATAFS_DIR
export UPDATERFS_DIR

TARGETS :=
TARGETS1 :=
TARGETS_CLEAN :=
TARGETS_DISTCLEAN :=

#########################################################################

ifeq ($(TOPDIR)/configs/config.mk,$(wildcard $(TOPDIR)/configs/config.mk))
ifeq ($(MOZART_DIR)/usr/include/mozart_config.h,$(wildcard $(MOZART_DIR)/usr/include/mozart_config.h))

include $(TOPDIR)/configs/config.mk

all: world rootfs

include configs/pkg.mk
include configs/mkfs.mk
include configs/*.mak

TARGETS_CLEAN:=$(patsubst %,%-clean,$(TARGETS1))
TARGETS_DISTCLEAN:=$(patsubst %,%-clean,$(TARGETS))

world: prepare $(TARGETS)

debug:
	@echo TOPDIR is $(TOPDIR)
	@echo TARGETS is $(TARGETS)

prepare: dirs

dirs: $(OUTPUT_DIR) $(TARGET_DIR) $(UPDATEPKG_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR)


$(OUTPUT_DIR) $(UPDATEPKG_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR):
	@mkdir -p $@
$(TARGET_DIR)::
	@rm -rf $@; mkdir -p $@
else
all world debug prepare dirs $(OUTPUT_DIR) $(TARGET_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR):
	@echo "output/mozart/usr/include/mozart_config.h Not Found,System not configured - see 'make help'" >&2
	@exit 1
endif
else
all world debug prepare dirs $(OUTPUT_DIR) $(TARGET_DIR) $(APPFS_DIR) $(MOZART_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR):
	@echo "configs/config.mk Not Found,System not configured - see 'make help'" >&2
	@exit 1
endif

###########################################################################
# function config
###########################################################################
mozart_prepare:
	@echo "#" > $(TOPDIR)/configs/config.mk
	@echo "# Automatically generated make config: don't edit" >> $(TOPDIR)/configs/config.mk
	@echo "# Mozart SDK configure" >> $(TOPDIR)/configs/config.mk
	@echo "#" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

	@mkdir -p $(MOZART_DIR)/usr/include
	@echo "/*" > $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " * Automatically generated C config: don't edit" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " * Mozart SDK configure" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo " */" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_NULL 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_RTK 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_BCM 2" >> $(MOZART_DIR)/usr/include/mozart_config.h



# wifi module config (SUPPORT_WIFI_MODULE=xxx) in config.mak
#   rtk_<module name>: use realtek's wifi module
#   bcm_<module name>: use broadcom's wifi module
#
# wifi smart config method (SUPPORT_WIFI_SC=xxx) in config.mak
#     realtek: use simple config protocol supported by Realtek
#    broadcom: use cooee protocol supported by Broadcom
#  airkiss_rtk: use airkiss(iot.weixin.qq.com) protocol supported by Wexin
#
# wifi module/chip config (#define SUPPORT_WIFI_MODULE xxx) in config.h
#   rtk_<module name>: use realtek's wifi module/chip
#   bcm_<module name>: use broadcom's wifi module/chip
wifi_rtl8723bs_config:
	@echo "SUPPORT_WIFI_MODULE=rtk_rtl8723bs" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=realtek" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"rtk_rtl8723bs\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6212_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6212\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6212a_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6212a\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6212a_atalk_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=atalk_broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6212a\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_ap6181_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_ap6181" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_ap6181\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_iw8101_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_iw8101" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_iw8101\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_bcm43438_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_bcm43438" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_bcm43438\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
wifi_bcm43362_config:
	@echo "SUPPORT_WIFI_MODULE=bcm_bcm43362" >> $(TOPDIR)/configs/config.mk
	@echo "SUPPORT_WIFI_SC=broadcom" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WIFI_MODULE \"bcm_bcm43362\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h


# bluetooth config in config.mak
#                   0: DO NOT support bluetooth
#   rtk_<module name>: use realtek's bt
#   bcm_<module name>: use broadcom's bt
#
# bluetooth config in config.h(XXX: need define those micro or enum manually in somewhere.)
#	BT_RTK:  use realtek's bt
#	BT_BCM:  use broadcom's bt
#	BT_NULL: disable
#	BT_???:  use ???'s bt
bt_rtl8723bs_config:
	@echo "SUPPORT_BT_MODULE=rtk_rtl8723bs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_RTK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_ap6212_config:
	@echo "SUPPORT_BT_MODULE=bcm_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_ap6212a_config:
	@echo "SUPPORT_BT_MODULE=bcm_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_bcm43438_config:
	@echo "SUPPORT_BT_MODULE=bcm_bcm43438" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_BCM" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
bt_disable_config:
	@echo "SUPPORT_BT_MODULE=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BT BT_NULL" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

bt_hci_dev_ttyS0_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS1_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS2_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
bt_hci_dev_ttyS3_config:
	@echo "SUPPORT_BT_HCI_DEV=/dev/ttyS3" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# mplayer config in config.mak
#   float: float point decoder
#   fixed: fixed point decoder
mplayer_float_config:
	@echo "SUPPORT_MPLAYER=float" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
mplayer_fixed_config:
	@echo "SUPPORT_MPLAYER=fixed" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# voice recognition config in config.mak and config.h
#       1: enable
#   other: disable
vr_micro_define_config:
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_NULL 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_BAIDU 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_SPEECH 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_IFLYTEK 3" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_UNISOUND 4" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_ATALK 5" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_baidu_config: vr_micro_define_config
	@echo "SUPPORT_VR=vr_baidu" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_BAIDU" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_speech_config: vr_micro_define_config
	@echo "SUPPORT_VR=vr_speech" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_SPEECH" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_iflytek_config: vr_micro_define_config
	@echo "SUPPORT_VR=vr_iflytek" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_IFLYTEK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_unisound_config: vr_micro_define_config
	@echo "SUPPORT_VR=vr_unisound" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_UNISOUND" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_atalk_config: vr_micro_define_config
	@echo "SUPPORT_VR=vr_atalk" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_ATALK" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_disable_config: vr_micro_define_config
	@echo "SUPPORT_VR=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR VR_NULL" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_wakeup_define_config:
	@echo "#define VR_WAKEUP_VOICE_KEY_MIX 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_VOICE 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_KEY_SHORTPRESS 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define VR_WAKEUP_KEY_LONGPRESS 3" >> $(MOZART_DIR)/usr/include/mozart_config.h

vr_wakeup_voice_key_mix_config: vr_wakeup_define_config
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_voice_key_mix" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_VOICE_KEY_MIX" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_voice_config: vr_wakeup_define_config
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_voice" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_VOICE" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_key_shortpress_config: vr_wakeup_define_config
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_key_shortpress" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_KEY_SHORTPRESS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
vr_wakeup_key_longpress_config: vr_wakeup_define_config
	@echo "SUPPORT_VR_WAKEUP=vr_wakeup_key_longpress" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_VR_WAKEUP VR_WAKEUP_KEY_LONGPRESS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h


# board type @yuanze.zhang 160907
# BOARD_PUBLIC_AIHOME 0
# BOARD_DS1825_ATALK　1
# BOARD_DS1825_AIHOME 2
# BOARD_DS1825_PRO 3
# BOARD_WB38_AIHOME 4 
board_type_define_config:
	@echo "/*--------------------------BOARD */" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BOARD_PUBLIC 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BOARD_DS1825 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BOARD_WB38 2" >> $(MOZART_DIR)/usr/include/mozart_config.h
board_public_config: board_type_define_config
	@echo "SUPPORT_BOARD=board_public" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BOARD BOARD_PUBLIC" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define DEVICE_NAME \"AIHOME\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_NAME \"AIHOME\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define AP_NAME DEVICE_NAME" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
board_ds1825_config: board_type_define_config
	@echo "SUPPORT_BOARD=board_ds1825" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BOARD BOARD_DS1825" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define DEVICE_NAME \"DS-1825\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_NAME \"DS-1825\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define AP_NAME DEVICE_NAME" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
board_wb38_config: board_type_define_config
	@echo "SUPPORT_BOARD=board_wb38" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_BOARD BOARD_WB38" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define DEVICE_NAME \"DS-WB38\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define BT_NAME \"DS-WB38\"" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define AP_NAME DEVICE_NAME" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# memory config in config.mak and config.h
#       32M: 
#   	64M: 
memory_type_define_config:
	@echo "/*--------------------------MEMORY */" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define MEMORY_32M 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define MEMORY_64M 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
memory_32m_config:
	@echo "SUPPORT_MEMORY=MEMORY_64M" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_MEMORY MEMORY_64M" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk
memory_64m_config:
	@echo "SUPPORT_MEMORY=MEMORY_64M" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_MEMORY MEMORY_64M" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk


# networking type @yuanze.zhang 160907
# ATALK_AIRKISS : 0
# COOEE_AIRKISS : 1
netowrk_define_config:
	@echo "/*--------------------------NETWORKING */" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define NETWORK_ATALK_AIRKISS 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "#define NETWORK_COOEE_AIRKISS 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
netowrk_atalk_airkiss_config: netowrk_define_config
	@echo "#define SUPPORT_NETWORK NETWORK_ATALK_AIRKISS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
netowrk_cooee_airkiss_config: netowrk_define_config
	@echo "#define SUPPORT_NETWORK NETWORK_COOEE_AIRKISS" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# lapsule config in config.mak and config.h
#       1: enable
#   other: disable
lapsule_enable_config:
	@echo "SUPPORT_LAPSULE=1" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LAPSULE 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk
lapsule_disable_config:
	@echo "SUPPORT_LAPSULE=0" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LAPSULE 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk

# elife config in config.mak and config.h
#       1: enable
#   other: disable
elife_enable_config:
	@echo "SUPPORT_ELIFE=1" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ELIFE 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk
elife_disable_config:
	@echo "SUPPORT_ELIFE=0" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ELIFE 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk

# aispeech config in config.mak and config.h
#       1: enable
#   other: disable
aispeech_enable_config:
	@echo "SUPPORT_AISPEECH=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AISPEECH 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
aispeech_disable_config:
	@echo "SUPPORT_AISPEECH=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AISPEECH 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# atalk config in config.mak and config.h
#       1: enable
#   other: disable
atalk_enable_config:
	@echo "SUPPORT_ATALK=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ATALK 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
atalk_disable_config:
	@echo "SUPPORT_ATALK=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_ATALK 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# aitalk config in config.mak and config.h
#       1: enable
#   other: disable
aitalk_enable_config:
	@echo "SUPPORT_AITALK=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AITALK 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
aitalk_disable_config:
	@echo "SUPPORT_AITALK=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AITALK 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
# dmr config in config.mak and config.h
#       1: enable
#   other: disable
dmr_enable_config:
	@echo "SUPPORT_DMR=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMR 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

dmr_disable_config:
	@echo "SUPPORT_DMR=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMR 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

webrtc_aec_enable_config:
	@echo "SUPPORT_WEBRTC=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WEBRTC 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

webrtc_aec_disable_config:
	@echo "SUPPORT_WEBRTC=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_WEBRTC 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# dms config in config.mak and config.h
#       1: enable
#   other: disable
dms_enable_config:
	@echo "SUPPORT_DMS=1" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMS 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk
dms_disable_config:
	@echo "SUPPORT_DMS=0" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_DMS 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(TOPDIR)/configs/config.mk

# lcd config in config.mak and config.h
#	1: enable
#	other: disable
lcd_enable_config:
	@echo "SUPPORT_LCD=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LCD 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
lcd_disable_config:
	@echo "SUPPORT_LCD=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LCD 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# smartui config in config.mak and config.h
#	1: enable
#	other: disable
smartui_enable_config:
	@echo "SUPPORT_SMARTUI=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SMARTUI 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
smartui_disable_config:
	@echo "SUPPORT_SMARTUI=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_SMARTUI 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# cgi config in config.mak and config.h
#	1: enable
#	other: disalbe
cgi_enable_config:
	@echo "SUPPORT_CGI=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_CGI 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h
cgi_disable_config:
	@echo "SUPPORT_CGI=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_CGI 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# airplay config in config.mak and config.h
#       1: enable
#   other: disable
airplay_enable_config:
	@echo "SUPPORT_AIRPLAY=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AIRPLAY 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

airplay_disable_config:
	@echo "SUPPORT_AIRPLAY=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_AIRPLAY 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h


# localplayer config in config.mak and config.h
#       1: enable
#   other: disable
localplayer_enable_config:
	@echo "SUPPORT_LOCALPLAYER=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LOCALPLAYER 1" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

localplayer_disable_config:
	@echo "SUPPORT_LOCALPLAYER=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
	@echo "#define SUPPORT_LOCALPLAYER 0" >> $(MOZART_DIR)/usr/include/mozart_config.h
	@echo "" >> $(MOZART_DIR)/usr/include/mozart_config.h

# adb debug support in config.mak
#       1: enable
#   other: disable
#XXX: set to 0 in release version!!!
adb_enable_config:
	@echo "SUPPORT_ADB_DEBUG=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
adb_disable_config:
	@echo "SUPPORT_ADB_DEBUG=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# new battery nv flag support in config.mak
#       1: enable
#   other: disable
battery_nv_flag_enable_config:
	@echo "SUPPORT_BATTERY_NV_FLAG=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
battery_nv_flag_disable_config:
	@echo "SUPPORT_BATTERY_NV_FLAG=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# rootfs type config in config.mak
#  ramdisk: build ramdisk rootfs
#     ext4: build ext4 rootfs
#   cramfs: build cramfs rootfs
rootfs_ramdisk_config:
	@echo "SUPPORT_FS=ramdisk" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
rootfs_ext4_config:
	@echo "SUPPORT_FS=ext4" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
rootfs_cramfs_config:
	@echo "SUPPORT_FS=cramfs" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# usrdata type config
#    jffs2: build jffs2 usrdata
#   others: disable
usrdata_jffs2_config:
	@echo "SUPPORT_USRDATA=jffs2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# legacy update config
#        1: enable
#   others: disable
update_legacy_enable_config:
	@echo "SUPPORT_UPDATE_LEGACY=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

update_legacy_disable_config:
	@echo "SUPPORT_UPDATE_LEGACY=0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# audio type config
#    oss : oss
#    alsa: alsa
audio_oss_config:
	@echo "SUPPORT_AUDIO=oss" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

audio_alsa_config:
	@echo "SUPPORT_AUDIO=alsa" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# filesystem optimize size config in config.mak
#        1: cutdown fs continous
#    other: DO NOT cutdown fs.
#XXX: Dangerous!!! set to 0 generally.
cutdown_fs_size:
	@echo "SUPPORT_CUT_CONTINUE=1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# cpu model define.
cpu_model_m150:
	@echo "CPU_MODEL=M150" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
cpu_model_x1000:
	@echo "CPU_MODEL=X1000" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# product name define.
#trunk baseboard v1.1 with rtl8723bs
product_trunk_v1.1_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#trunk baseboard v1.1 with ap6212
product_trunk_v1.1_ap6212_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.1_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#trunk baseboard v1.0 with rtl8723bs
product_trunk_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.0" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#trunk baseboard v1.0 with ap6212
product_trunk_ap6212_config: cpu_model_m150
	@echo "PRODUCT_NAME=trunk_v1.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#ga1 baseboard with ap6212
product_ga1_ap6212_config: cpu_model_m150
	@echo "PRODUCT_NAME=ga1" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#phoenix board v1.0 with ap6212
product_phoenix_v1.0_ap6212_config: cpu_model_x1000
	@echo "PRODUCT_NAME=phoenix_v1.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#aslmom coreboard v1.0 with ap6212a
product_aslmom_v1.0_ap6212a_config: cpu_model_x1000
	@echo "PRODUCT_NAME=aslmom_v1.0_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v1.0 with ap6212a
product_canna_v1.0_ap6212a_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v1.0_ap6212a" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#canna coreboard v2.0 with ap6212
product_canna_v2.0_ap6212_config: cpu_model_x1000
	@echo "PRODUCT_NAME=canna_v2.0_ap6212" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#mensa board
product_mensa_config:
	@echo "PRODUCT_NAME=mensa" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk
#maf1079 board
product_maf1079_config:
	@echo "PRODUCT_NAME=maf1079" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk


# audio device info.
audio_dev_m150_config:
	@echo "AUDIO_DEV_PLAYBACK=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_RECORD=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_PCM=/dev/dsp1" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_SPDIFOUT=/dev/dsp2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

audio_dev_x1000_config:
	@echo "AUDIO_DEV_PLAYBACK=/dev/dsp" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_RECORD=/dev/dsp3" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_PCM=/dev/dsp1" >> $(TOPDIR)/configs/config.mk
	@echo "AUDIO_DEV_SPDIFOUT=/dev/dsp2" >> $(TOPDIR)/configs/config.mk
	@echo "" >> $(TOPDIR)/configs/config.mk

# flash info define.
#  spi nor we used now:
#      gd25q128c: 16MB
#      gd25lq64c: 8MB
#  pars:
#      USRDATA_FLASH_PAGESIZE: flash page size
#     USRDATA_FLASH_ERASESIZE: flash earse size
#       USRDATA_FLASH_PADSIZE: usrdata partition size
flash_spi_gd25q256c_config:
	@echo "FLASH_TYPE=spinor" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x100" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_PADSIZE=0xa0000" >> $(TOPDIR)/configs/config.mk #640KB

flash_spi_gd25q128c_config:
	@echo "FLASH_TYPE=spinor" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x100" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_PADSIZE=0xa0000" >> $(TOPDIR)/configs/config.mk #640KB

flash_spi_gd25lq64c_config:
	@echo "FLASH_TYPE=spinor" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_PAGE_SIZE=0x100" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_USRDATA_PADSIZE=0x38000" >> $(TOPDIR)/configs/config.mk #224KB

flash_emmc_config:
	@echo "FLASH_TYPE=emmc" >> $(TOPDIR)/configs/config.mk
	@echo "FLASH_ERASE_BLOCK_SIZE=0x8000" >> $(TOPDIR)/configs/config.mk # virtual setting, ONLY used for padding nv.img

###########################################################################
#
# board config
#
###########################################################################
trunk_v1.1_ap6212_cramfs_config: unconfig mozart_prepare product_trunk_v1.1_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS1_config \
		lcd_disable_config webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_disable_config \
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for trunk v1.1 ap6212 board"


trunk_v1.1_ap6212_mini_cramfs_config: unconfig mozart_prepare product_trunk_v1.1_ap6212_config \
		wifi_ap6212_config bt_disable_config webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		lcd_disable_config \
		mplayer_float_config vr_disable_config \
		lapsule_disable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config mini sdk with cramfs for trunk v1.1 ap6212 board"


trunk_cramfs_config: unconfig mozart_prepare product_trunk_config \
		wifi_rtl8723bs_config bt_disable_config webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		lcd_disable_config \
		mplayer_float_config vr_disable_config \
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for trunk board."


trunk_mini_cramfs_config: unconfig mozart_prepare product_trunk_config \
		wifi_rtl8723bs_config bt_disable_config webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		lcd_disable_config \
		mplayer_float_config vr_disable_config \
		lapsule_disable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config mini sdk with cramfs for trunk board."

aslmom_v1.0_ap6212a_cramfs_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		lcd_enable_config webrtc_aec_disable_config \
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for aslmom v1.0 ap6212a board"

#aslmom_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
#		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
#		smartui_enable_config webrtc_aec_enable_config\
#		audio_dev_x1000_config dmr_enable_config dms_disable_config \
#		airplay_enable_config localplayer_enable_config \
#		mplayer_float_config vr_atalk_config vr_wakeup_key_shortpress_config \
#		atalk_enable_config adb_enable_config battery_nv_flag_enable_config\
#		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
#	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

aslmom_v1.0_ap6212a_cramfs_atalk_speech_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		atalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

ds1825_atalk_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_disable_config elife_disable_config \
		board_ds1825_config netowrk_atalk_airkiss_config aispeech_disable_config\
		mplayer_float_config vr_atalk_config vr_wakeup_voice_config \
		atalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for ds1825 v1.0 ap6212a board"

ds1825_aispeech_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_disable_config elife_disable_config \
		board_ds1825_config memory_type_define_config memory_64m_config \ 
		netowrk_atalk_airkiss_config aispeech_enable_config\
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		aitalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for ds1825 v1.0 ap6212a board"

ds1825_aispeech_elife_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_enable_config elife_enable_config \
		board_ds1825_config memory_type_define_config memory_64m_config \
		netowrk_atalk_airkiss_config aispeech_enable_config\
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		aitalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for ds1825 v1.0 ap6212a board"

wb38_aispeech_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_disable_config elife_disable_config \
		board_wb38_config memory_type_define_config memory_64m_config \
		netowrk_atalk_airkiss_config aispeech_enable_config \
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		aitalk_enable_config adb_enable_config battery_nv_flag_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for ds1825 v1.0 ap6212a board"

wb38_aispeech_elife_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_disable_config elife_enable_config \
		board_wb38_config memory_type_define_config memory_64m_config \
		netowrk_atalk_airkiss_config aispeech_enable_config\
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		aitalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for elife v1.0 ap6212a board"

public_aispeech_v1.0_ap6212a_cramfs_atalk_config: unconfig mozart_prepare product_aslmom_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_disable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		cgi_disable_config elife_disable_config \
		board_public_config memory_type_define_config memory_32m_config \
		netowrk_atalk_airkiss_config aispeech_enable_config\
		mplayer_float_config vr_speech_config vr_wakeup_voice_config \
		aitalk_enable_config adb_enable_config battery_nv_flag_enable_config\
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for ds1825 v1.0 ap6212a board"
	
canna_v1.0_ap6212a_cramfs_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		lcd_disable_config webrtc_aec_disable_config \
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_speech_config vr_wakeup_voice_key_mix_config\
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

canna_v1.0_ap6212a_cramfs_atalk_update_legacy_config: unconfig mozart_prepare product_canna_v1.0_ap6212a_config \
		wifi_ap6212a_atalk_config bt_ap6212a_config bt_hci_dev_ttyS0_config \
		smartui_enable_config webrtc_aec_enable_config\
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_atalk_config vr_wakeup_key_shortpress_config \
		atalk_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config \
		update_legacy_enable_config
	@echo "Config complete sdk with cramfs for canna v1.0 ap6212a board"

canna_v2.0_43438_cramfs_config: unconfig mozart_prepare product_canna_v2.0_ap6212_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config \
		lcd_disable_config webrtc_aec_disable_config \
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_speech_config vr_wakeup_key_shortpress_config \
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for canna v2.0 ap6212 board"

phoenix_v1.0_ap6212_cramfs_config: unconfig mozart_prepare product_phoenix_v1.0_ap6212_config \
		lcd_disable_config webrtc_aec_disable_config \
		wifi_ap6212_config bt_ap6212_config bt_hci_dev_ttyS0_config \
		audio_dev_x1000_config dmr_enable_config dms_disable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_disable_config \
		lapsule_enable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config complete sdk with cramfs for phoenix v1.0 ap6212 board"


ga1_config: unconfig mozart_prepare product_ga1_ap6212_config \
		wifi_ap6212a_config bt_ap6212a_config bt_hci_dev_ttyS1_config \
		lcd_disable_config webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_disable_config \
		lapsule_disable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config mini sdk with cramfs for ga1 board"


maf1079_config: unconfig mozart_prepare product_maf1079_config \
		wifi_bcm43362_config bt_disable_config \
		lcd_disable_defconfig webrtc_aec_disable_config \
		audio_dev_m150_config dmr_enable_config dms_enable_config \
		airplay_enable_config localplayer_enable_config \
		mplayer_float_config vr_disable_config \
		lapsule_disable_config adb_enable_config \
		rootfs_cramfs_config usrdata_jffs2_config flash_spi_gd25q128c_config
	@echo "Config mini sdk with cramfs for maf1079 board."


unconfig:
	@echo "you are applying a new configure..."
	@echo -e $(shell test -d $(APPFS_DIR) && echo "You should run \'make clean\' to compile mozart in a clean environment")
	@echo

help:
	@echo "Configs:"
	@echo "    make <user>_config                      - make <user>'s config"
	@echo "                                              Should apply a valid config before build!!"
	@echo
	@echo "built-in configs:"
	@echo "    trunk_cramfs_config                     - a full function version fot trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY, VOICE-RECOGNITION"
	@echo
	@echo "    trunk_mini_cramfs_config                - a cutdown version for trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY"
	@echo
	@echo "    trunk_v1.1_ap6212_cramfs_config         - a cutdown version for trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY, BT, VOICE-RECOGNITION"
	@echo
	@echo "    trunk_v1.1_ap6212_mini_cramfs_config    - a cutdown version for trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY"
	@echo
	@echo "    ga1_config                              - a cutdown version for ga1 board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY"
	@echo
	@echo "    canna_v1.0_ap6212a_cramfs_config        - a cutdown version for trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY, BT"
	@echo
	@echo "    phoenix_v1.0_ap6212_cramfs_config       - a cutdown version for trunk board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY, BT"
	@echo
	@echo "    maf1079_config                          - a cutdown version for maf1079 board."
	@echo "                                              support DLNA, AIRPLAY, LOCALPLAY"
	@echo "Others:"
	@echo "    clean                                   - invoke main-clean app-config-clean updater-clean"
	@echo "    distclean                               - clean all targets"


clean: $(TARGETS_CLEAN)
	-rm -rf $(OUTPUT_DIR)/fs $(TARGET_DIR)

distclean: $(TARGETS_DISTCLEAN) clean
	-rm -rf $(MOZART_DIR) $(MOZART_UPDATER_DIR) $(OUTPUT_DIR)/host

show-targets:
	@echo "TOPDIR is $(TOPDIR)"
	@echo
	@echo "TARGETS(clean, distclean) is $(TARGETS)"
	@echo
	@echo "TARGETS1(clean) is $(TARGETS1)"
	@echo

.PHONY: all world dirs clean distclean \
	$(TARGETS) $(TARGETS_ALL) \
	$(TARGETS_DISTCLEAN) $(APPFS_DIR) $(USRDATAFS_DIR) $(UPDATERFS_DIR)
