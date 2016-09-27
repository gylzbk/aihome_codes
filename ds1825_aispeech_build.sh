#!/bin/bash

rm ./update/nv.img
rm ./update/appfs.cramfs
rm ./update/updater.cramfs
rm ./update/usrdata.jffs2

local=`pwd`
VERSION=ds1825_aispeech_v0.2.0
MOZART_PATH=${local}/mozart/


#---------------------------------UBOOT
#./uboot_build.sh

#---------------------------------KERNEL
#./kernel_build.sh

#---------------------------------mozart
cd ${MOZART_PATH}
make clean
make distclean
make ds1825_aispeech_v1.0_ap6212a_cramfs_atalk_config
#make libaiserver-rebuild -j4
#make libaispeech-rebuild -j4
#make mozart-rebuild -j4
make -j4
#./rebuild.sh

cp output/target/appfs.cramfs   ${local}/update
cp output/target/updater.cramfs ${local}/update
cp output/target/usrdata.jffs2  ${local}/update
cp output/target/nv.img ${local}/update

cd ${local}
date_now=$(date +%Y%m%d_%H%M) #今天的日期
filename=update_${VERSION}_${date_now}.tar.bz2
tar -cjvf ${filename} update # ui atalk-favorite
mv ${filename} ./version/

echo "============================== build all ok !"
date 
