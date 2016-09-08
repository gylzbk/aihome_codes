#!/bin/bash
dir=`pwd`
version=$1

if [ $# -lt 1 ]
then
    echo "please ./mkzip.sh 20150808"
    exit -1
fi


echo version=$version >> info

kernel_updata_flag=0

if [ ! -f $dir/updatezip/image/zImage ]
then
	echo "not found $dir/updatezip/image/zImage"
	kernel_updata_flag=1
fi
if [ ! -f $dir/updatezip/image/updater.cramfs ]
then
	echo "not found $dir/updatezip/image/updater.cramfs"
	kernel_updata_flag=`expr $kernel_updata_flag + 10`
fi


if [ ! -f $dir/updatezip/image/appfs.cramfs ]
then
	echo "not found $dir/updatezip/image/appfs.cramfs"
	echo no usrfs.img  can not update
	echo -1
fi

if [ $kernel_updata_flag -eq 11 ]
then
	mode=1
elif [ $kernel_updata_flag -eq 0 ]
then
	mode=0
else
	echo you are wrong!!! we do not update for you!
	exit -1
fi

echo mode=$mode >> info






split -b 1M -d $dir/updatezip/image/zImage  $dir/updatezip/split/kernel_
split -b 1M -d $dir/updatezip/image/updater.cramfs  $dir/updatezip/split/update_
split -b 1M -d $dir/updatezip/image/appfs.cramfs  $dir/updatezip/split/usrfs_
cd $dir


files_kernel=`ls $dir/updatezip/split/kernel_*`
files_update=`ls $dir/updatezip/split/update_*`
files_usrfs=`ls $dir/updatezip/split/usrfs_*`

totalfiles_kernel=0
totalfiles_update=0
totalfiles_usrfs=0


for f in $files_kernel
do
		    totalfiles_kernel=`expr $totalfiles_kernel + 1`
done
echo "totalfiles_kernel is " $totalfiles_kernel


for f in $files_update
do
		    totalfiles_update=`expr $totalfiles_update + 1`
done
echo "totalfiles_update is " $totalfiles_update


for f in $files_usrfs
do
		    totalfiles_usrfs=`expr $totalfiles_usrfs + 1`
done
echo "totalfiles_usrfs is " $totalfiles_usrfs


sizeof_kernel=`ls -l $dir/updatezip/image/zImage | awk '{ print $5 }'`
sizeof_updatefs=`ls -l $dir/updatezip/image/updater.cramfs | awk '{ print $5 }'`
sizeof_usrfs=`ls -l $dir/updatezip/image/appfs.cramfs | awk '{ print $5 }'`



for i in $(seq 0 `expr $totalfiles_kernel + $totalfiles_update + $totalfiles_usrfs - 1`)
do
#	echo `expr $totalfiles_kernel + $totalfiles_update + $totalfiles_usrfs - 1`


if [ $i -lt $totalfiles_kernel ]

then
	echo "echo sum=$(expr $totalfiles_kernel + $totalfiles_update + $totalfiles_usrfs) > /tmp/tmpfile" > $dir/updatezip/split/script/update.script
echo "echo reboot=0 >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo kernel_size=$sizeof_kernel >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo update_size=$sizeof_updatefs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo usrfs_size=$sizeof_usrfs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "dd if=/tmp/updatezip$i/image/kernel_0$i of=/dev/mtdblock4 seek=$[ 1024 * $i + 512*9 ] bs=1024 count=1024" >> $dir/updatezip/split/script/update.script

elif [ $i -gt `expr $totalfiles_kernel + $totalfiles_update - 1` ]
then
	echo "echo sum=$(expr $totalfiles_kernel + $totalfiles_update + $totalfiles_usrfs) > /tmp/tmpfile" > $dir/updatezip/split/script/update.script
echo "echo reboot=0 >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo kernel_size=$sizeof_kernel >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo update_size=$sizeof_updatefs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo usrfs_size=$sizeof_usrfs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
if [ $i -eq $(expr $totalfiles_kernel + $totalfiles_update) ]
	then
		echo "echo mtd_debug erase /dev/mtd4 0 0x800000 >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
	fi
echo "dd if=/tmp/updatezip$i/image/usrfs_0$(expr $i - $totalfiles_kernel - $totalfiles_update) of=/dev/mtdblock4 seek=$[ 1024 * $(expr $i - $totalfiles_kernel - $totalfiles_update) ] bs=1024 count=1024" >> $dir/updatezip/split/script/update.script

else
	echo "echo sum=$(expr $totalfiles_update + $totalfiles_kernel + $totalfiles_usrfs) > /tmp/tmpfile" > $dir/updatezip/split/script/update.script
	if [ $i -eq $(expr $totalfiles_kernel + $totalfiles_update - 1) ]
	then
		echo "echo reboot=1 >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
	else
		echo "echo reboot=0 >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
	fi
echo "echo kernel_size=$sizeof_kernel >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo update_size=$sizeof_updatefs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "echo usrfs_size=$sizeof_usrfs >> /tmp/tmpfile" >> $dir/updatezip/split/script/update.script
echo "dd if=/tmp/updatezip$i/image/update_0$(expr $i - $totalfiles_kernel ) of=/dev/mtdblock4 seek=$[ 1024 * $(expr $i - $totalfiles_kernel) ] bs=1024 count=1024" >> $dir/updatezip/split/script/update.script

fi



cd $dir/updatezip/split/
mkdir updatezip$i
cd updatezip$i
mkdir image
cd ..
cp -r script updatezip$i
if [ $i -lt $totalfiles_kernel ]
then
	echo kernel
	cp kernel_0$i updatezip$i/image
elif [ $i -gt `expr $totalfiles_kernel + $totalfiles_update - 1` ]
then
	echo usrfs
	echo $(expr $i - $totalfiles_kernel - $totalfiles_update)
	cp usrfs_0$(expr $i - $totalfiles_kernel - $totalfiles_update) updatezip$i/image
else
	echo update
	echo $(expr $i - $totalfiles_kernel)
	cp update_0$(expr $i - $totalfiles_kernel) updatezip$i/image
fi

echo ----- Making update.zip ------
zip -r ../../update$i.zip updatezip$i
echo ----- Made update.zip --------



cd $dir
#md5sum update$i.zip > update$i.zip.md5
md5sum update$i.zip >> info_$version


rm -rf $dir/updatezip/split/updatezip$i
rm -rf $dir/updatezip/split/image/
i=$(($i+1))
done

cd $dir

if [ -e output ]; then
	rm -rf output
fi

mkdir -p output
mkdir output/update_$version
cp *.zip output/update_$version/
rm *.zip
cd updatezip/split
rm kernel_*
rm update_*
rm usrfs_*
cd ../..

mv info_$version output/update_$version
mv info output/
