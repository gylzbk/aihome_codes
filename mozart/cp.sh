#!/bin/bash
#cp output/target/appfs.cramfs output/target/usrdata.jffs2 output/target/updater.cramfs /mnt/tao/Documents/Y_customers/A_Ingenic/image/x1000_canna/9_14/thread
if [ $# -eq 1 ];then
	cp output/target/appfs.cramfs output/target/usrdata.jffs2 output/target/updater.cramfs output/target/nv.img $1
#	cp tools/host-tools/update_pack/source/image/u-boot-with-spl.bin $1
#	cp tools/host-tools/update_pack/source/image/zImage $1
else
	echo "Usage: cp.sh [path]"
fi
