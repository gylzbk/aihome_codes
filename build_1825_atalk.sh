#!/bin/bash

#rm ./update/README.txt
#rm ./update/u-boot-with-spl.bin
#rm ./update/zImage
rm ./update/nv.img
rm ./update/appfs.cramfs
rm ./update/updater.cramfs
rm ./update/usrdata.jffs2

local=`pwd`
VERSION=ds1825pro_v0.2.0
UBOOT_PATH=/opt/ingenic/x1000/doss/codes/uboot-doss/
KERNEL_PATH=/opt/ingenic/x1000/doss/codes/kernel-3.0.8-doss/
MOZART_PATH=/opt/ingenic/x1000/doss/codes/mozart/

#touch /update/README.txt


#---------------------------------UBOOT
cd ${UBOOT_PATH}
make clean
make disclean
make aslmom_v10_zImage_sfc_nor
cp u-boot-with-spl.bin ${local}/update

#---------------------------------KERNEL
cd ${KERNEL_PATH}
make clean
make disclean
#make aslmom_v10_oss_defconfig
make ds1825_resample_defconfig
make zImage -j4
cp arch/mips/boot/compressed/zImage ${local}/update

#---------------------------------mozart
cd ${MOZART_PATH}
make clean
make distclean
make ds1825_atalk_v1.0_ap6212a_cramfs_atalk_config
make libaiserver-rebuild -j4
make libaispeech-rebuild -j4
make mozart-rebuild -j4
make -j4
./rebuild.sh

cp output/target/appfs.cramfs   ${local}/update
cp output/target/updater.cramfs ${local}/update
cp output/target/usrdata.jffs2  ${local}/update
cp output/target/nv.img ${local}/update

cd ${local}
date_now=$(date +%Y%m%d_%H%M) #今天的日期
filename=update_${VERSION}_${date_now}.tar.bz2
tar -cjvf ${filename} update ui atalk-favorite
mv ${filename} ./version

echo "============================== build all ok !"
date 
