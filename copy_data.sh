#!/bin/bash

TOP_DIR1=$(pwd)

uboot_flag=0
kernel_flag=0
mozart_flag=0

if [ ${TOP_DIR1##*/} = "uboot-doss" ]; then
	uboot_flag=1
fi
if [ ${TOP_DIR1##*/} = "kernel-3.0.8-doss" ]; then
	kernel_flag=1
fi
if [ ${TOP_DIR1##*/} = "mozart" ]; then
	mozart_flag=1
fi

if [ ${uboot_flag} = "0" -a ${mozart_flag} = "0" -a ${mozart_flag} = "0" ]; then
	make_img=1	
	TOP_DIR=${TOP_DIR1}
fi

if [ "$make_img" = "1" ]; then
	UBOOT_DIR=${TOP_DIR}/uboot-doss
	KERNEL_DIR=${TOP_DIR}/kernel-3.0.8-doss
	MOZART_DIR=${TOP_DIR}/mozart
else

	UBOOT_DIR=${TOP_DIR1}
	KERNEL_DIR=${TOP_DIR1}
	MOZART_DIR=${TOP_DIR1}
fi

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

	cp ./u-boot-with-spl.bin ../firmware/img
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
	
	cp ./arch/mips/boot/compressed/zImage ../firmware/img
	cp ./arch/mips/boot/compressed/zImage ../mozart/tools/host-tools/update_pack/images/
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

	cp  ./output/target/nv.img		../firmware/img
	cp  ./output/target/usrdata.jffs2	../firmware/img
	cp  ./output/target/updater.cramfs	../firmware/img
	cp  ./output/target/appfs.cramfs	../firmware/img
fi

echo "------------------------------"
date
echo "successful ok                 "
echo "------------------------------"

