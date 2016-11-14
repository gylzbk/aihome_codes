#ifndef __A_H__
#define __A_H__

#define OTAFILE_CNT(arr) (sizeof(arr) / sizeof((arr)[0]))

enum {
	OTAFILE_UPDATER = 0,
	OTAFILE_UBOOT,
	OTAFILE_USRDATA,
	OTAFILE_KERNEL,
	OTAFILE_APPFS,
};

struct otafile_info {
	char name[32];
	char blkdev[16];
	long long offset;
	long long size;
};

struct otafile {
	char magic[8]; // magic number: OTA
	int cnt;
	struct otafile_info *info;
};

#endif //__A_H__
