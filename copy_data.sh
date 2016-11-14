#!/bin/bash

TOP_DIR=$(pwd)

UBOOT_DIR=${TOP_DIR}/uboot-doss
KERNEL_DIR=${TOP_DIR}/kernel-3.0.8-doss
MOZART_DIR=${TOP_DIR}/mozart

if [ "$1" = "uboot" ]; then
	cd ${UBOOT_DIR}
	if [ "$2" = "" ]; then
		make -j4
	elif [ "$2" = "wb38" ]; then
		make distclean
		make doss_wb38_zImage_sfc_nor -j4
	elif [ "$2" = "1825" ]; then
		make distclean
		make doss_1825_zImage_sfc_nor -j4
	fi

	cp ${UBOOT_DIR}/u-boot-with-spl.bin ${TOP_DIR}/firmware/img
fi

if [ "$1" = "kernel" ]; then
	cd ${KERNEL_DIR}
	if [ "$2" = "" ]; then
		make zImage -j4
	elif [ "$2" = "wb38" ]; then
		make distclean
		make doss_wb38_defconfig
		make zImage -j4
	fi
	
	cp ${KERNEL_DIR}/arch/mips/boot/compressed/zImage ${TOP_DIR}/firmware/img
	cp ${KERNEL_DIR}/arch/mips/boot/compressed/zImage ${MOZART_DIR}/tools/host-tools/update_pack/images/
fi

if [ "$1" = "mozart" ]; then
	cd ${MOZART_DIR}
	if [ "$2" = "" ]; then
		make mozart-clean
		make libaispeech-clean
		make libaiserver-clean
		make -j4
	elif [ "$2" = "wb38" ]; then
		make distclean
		make wb38_aispeech_v1.0_ap6212a_cramfs_atalk_config
		make -j4
	elif [ "$2" = "1825" ]; then
		make distclean
		make ds1825_aispeech_v1.0_ap6212a_cramfs_atalk_config
		make -j4
	elif [ "$2" = "1825-elife" ]; then
		make distclean
		make ds1825_aispeech_elife_v1.0_ap6212a_cramfs_atalk_config
		make -j4
	fi

	cp  ${MOZART_DIR}/output/target/nv.img	 ${TOP_DIR}/firmware/img
	cp  ${MOZART_DIR}/output/target/usrdata.jffs2	 ${TOP_DIR}/firmware/img
	cp  ${MOZART_DIR}/output/target/updater.cramfs ${TOP_DIR}/firmware/img
	cp  ${MOZART_DIR}/output/target/appfs.cramfs	 ${TOP_DIR}/firmware/img
fi

echo "------------------------------"
date
echo "successful ok                 "
echo "------------------------------"
echo 
