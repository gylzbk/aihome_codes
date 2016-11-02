#include <stdio.h>

#include "otafile.h"

static struct otafile_info info[] = {
	[OTAFILE_UPDATER] = {
		.name = "updater",
		.blkdev = {},
		.offset = -1,
		.size = -1,
	},
	[OTAFILE_UBOOT] = {
		.name = "uboot",
		.blkdev = {},
		.offset = -1,
		.size = -1,
	},
	[OTAFILE_KERNEL] = {
		.name = "kernel",
		.blkdev = {},
		.offset = -1,
		.size = -1,
	},
	[OTAFILE_APPFS] = {
		.name = "app",
		.blkdev = {},
		.offset = -1,
		.size = -1,
	},
	[OTAFILE_USRDATA] = {
		.name = "usrdata",
		.blkdev = {},
		.offset = -1,
		.size = -1,
	},
};

struct otafile f = {
	.magic = "OTAFILE",
	.cnt = OTAFILE_CNT(info),
	.info = info,
};
