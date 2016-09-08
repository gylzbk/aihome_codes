#! /usr/bin/env bash

gen_info_file()
{
	OUT=$2/info.ini
	DATE=`date +"%a, %d %b %Y %T %Z"`
	SIZE=`du -s $3 | cut -f1`

	echo "gen $OUT"

	echo -e	"[UpdateInfo]\nversion=$1\ntime=$DATE\nsize=$SIZE" > $OUT
	echo
	cat $OUT
	echo
}

gen_info_version_file()
{
	echo "gen `dirname $2`/info_$1"
	md5sum $2 > `dirname $2`/info_$1
}

gen_clear()
{
	OUT=$1
	rm -rf $OUT/temp
}

gen_usage()
{
	echo "Usage: `basename $0` <version> [outputdir] [sourceimage]"
	echo "Example:"
	echo "       `basename $0` v1.00001"
	echo "       `basename $0` v1.00001 foo foo"
}

if [ $# -lt 1 ]; then
	gen_usage
	exit 1
fi

if [[ $1 =~ ^[vV][0-9]+\.[0-9]{1,5}$ ]]; then
	VERSION=$(echo $1 | sed 's/[a-zA-Z]*//' | awk '{printf "%.5f", $1}')
	VERSION="v$VERSION"
else
	echo "Uncorrect version format"
	gen_usage
	exit 1
fi

PREFIX=`dirname $0`/..
CURRENT=$PWD

SOURCE=$PREFIX/source

if [ -z $2 ]; then
	OUTPUT=$PREFIX/output
else
	OUTPUT=$2
fi

if [ -n $3 ]; then
	if [ ! -d $3 ]; then
		echo "Directory $3 is NOT exist"
		exit 1
	fi

	IMAGE=$3
fi

if [ -d $OUTPUT ]; then
	# Clear output
	rm -rf $OUTPUT/*
else
	mkdir $OUTPUT
fi

if [ ! -f "$SOURCE/update_sys/zImage-ramfs" ]; then
	echo "zImage-ramfs MUST needed"
	exit 1
fi

mkdir -p $OUTPUT/temp/update

cp $SOURCE/update_sys/zImage-ramfs $OUTPUT/temp
cp -a $SOURCE/image/* $OUTPUT/temp/update

if [ -n $IMAGE ]; then
	cp -a $IMAGE/* $OUTPUT/temp/update
fi

mkdir $OUTPUT/update_$VERSION

# Goto temp dir
cd $OUTPUT/temp
echo ""
echo "==================== md5sum ===================="
md5sum update/*
echo ""
zip -r9 update.zip update zImage-ramfs
if [ ! $? ]; then
	echo "Greate OTA update zip package failed."
	gen_clear $OUTPUT
	cd $CURRENT
	exit 1
fi

cp update.zip ../update_$VERSION

cd ../update_$VERSION
gen_info_version_file $VERSION update.zip

# Return
cd $CURRENT

# Gen update.ini
gen_info_file $VERSION $OUTPUT $OUTPUT/update_$VERSION/update.zip

gen_clear $OUTPUT

echo "Create OTA packages success."
exit 0
