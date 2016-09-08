#!/bin/bash
if [ $# -eq 1 ];then
	cd output/target/updatepkg && zip -r updatepkg.zip * && cd -
	echo "cp updaepkg.zip $1"
	cp output/target/updatepkg/updatepkg.zip $1	
else
	echo "Usage: get_updater.sh [path]"
fi
