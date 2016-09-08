#! /usr/bin/env bash

gen_ini_file()
{
	OUT=$2/update.ini

	echo "gen $OUT"

	echo -e	"[Update]\nversion=$1" > $OUT
	echo
	cat $OUT
}

gen_usage()
{
	echo "Usage: `basename $0` <version>"
	echo "Example:"
	echo "       `basename $0` v1.00001"
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

SOURCE=$PREFIX/source
OUTPUT=$PREFIX/output

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

# Gen update.ini
gen_ini_file $VERSION $OUTPUT

cp $SOURCE/update_sys/zImage-ramfs $OUTPUT

mkdir $OUTPUT/update

cp -a $SOURCE/image/* $OUTPUT/update

if [ $? ]; then
	echo "Create sd update success"
else
	echo "Greate sd update failed"
	exit 1
fi
