#!/bin/bash

export TOP_DIR=$(pwd)
export make_img=1

source ${TOP_DIR}/mozart/configs/version.mk

if [ "$1" = "all" ]; then
	cd ${TOP_DIR}
	source ./copy_data.sh uboot 1825
	cd ${TOP_DIR}
	source ./copy_data.sh kernel 1825 

	cd ${TOP_DIR}
	source ./copy_data.sh mozart 1825
fi

if [ "$1" = "" ]; then
	cd ${TOP_DIR}
	source ./copy_data.sh mozart
fi

cd ${TOP_DIR}/firmware
DATE=$(date +%Y%m%d) #今天的日期
filename=ds1825_${MOZART_VERSION}_${DATE}.tar.bz2
tar -cjvf ${filename} img/ # ui atalk-favorite
mv ${filename} version/

echo "-------------------------------"
echo "generate release package:      "
echo "    ${filename}"
echo "-------------------------------"
