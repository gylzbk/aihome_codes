#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "flash.h"
#include "debug.h"
#include "ini_interface.h"

typedef enum flash_type {
	SPINOR = 1,
	SPINAND,
	NAND,
	EMMC,
	UNKNOWN,
} flash_type;

flash_type flash_get_type(void)
{
	int ret = 0;
	char buf[32] = {};

	ret = mozart_ini_getkey("/usr/data/system.ini", "nv", "storage", buf);
	if (ret)
		return UNKNOWN;

	if (!strcmp(buf, "spinor"))
		return SPINOR;
	else if (!strcmp(buf, "spinand"))
		return SPINAND;
	else if (!strcmp(buf, "nand"))
		return NAND;
	else if (!strcmp(buf, "emmc"))
		return EMMC;
	else
		return UNKNOWN;

	return UNKNOWN;
}

void *flash_read(char *blkname, off_t offset, ssize_t len)
{
	void *ret = NULL;
	flash_type type = UNKNOWN;

	type = flash_get_type();
	switch (type) {
	case SPINOR:
		ret = spinor_read(blkname, offset, len);
		break;
	case SPINAND:
		ret = spinand_read(blkname, offset, len);
		break;
	case NAND:
		break;
	case EMMC:
		ret = emmc_read(blkname, offset, len);
		break;
	case UNKNOWN:
	default:
		printf("flash type unknown.\n");
		break;
	}

	return ret;
}

int flash_earse(char *blkname, off_t offset, ssize_t len)
{
	int ret = 0;
	flash_type type = UNKNOWN;
	type = flash_get_type();

	switch (type) {
	case SPINOR:
		ret = spinor_earse(blkname, offset, len);
		break;
	case SPINAND:
		ret = spinand_earse(blkname, offset, len);
		break;
	case NAND:
		ret = nand_earse(blkname, offset, len);
		break;
	case EMMC:
		ret = emmc_earse(blkname, offset, len);
		break;
	case UNKNOWN:
	default:
		printf("flash type unknown.\n");
		ret = -1;
		break;
	}

	return ret;
}

int flash_write(char *blkname, off_t offset, void *data, ssize_t len)
{
	int ret = 0;
	flash_type type = UNKNOWN;
	type = flash_get_type();

	switch (type) {
	case SPINOR:
		ret = spinor_write(blkname, offset, data, len);
		break;
	case SPINAND:
		ret = spinand_write(blkname, offset, data, len);
		break;
	case NAND:
		ret = nand_write(blkname, offset, data, len);
		break;
	case EMMC:
		ret = emmc_write(blkname, offset, data, len);
		break;
	case UNKNOWN:
	default:
		printf("flash type unknown.\n");
		ret = -1;
		break;
	}

	return ret;
}

int flash_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size)
{
	int ret = 0;
	flash_type type = UNKNOWN;
	type = flash_get_type();

	switch (type) {
	case SPINOR:
		ret = spinor_write_file(blkname, file, seek, skip, size);
		break;
	case SPINAND:
		ret = spinand_write_file(blkname, file, seek, skip, size);
		break;
	case NAND:
		ret = nand_write_file(blkname, file, seek, skip, size);
		break;
	case EMMC:
		ret = emmc_write_file(blkname, file, seek, skip, size);
		break;
	case UNKNOWN:
	default:
		printf("flash type unknown.\n");
		ret = -1;
		break;
	}

	return ret;
}

size_t flash_size_get(char *blkname)
{
	int ret = -1;
	int fd = -1;
	size_t dev_size = -1;

	if (!blkname)
		return -1;

	fd = open(blkname, O_RDONLY);

	if (fd < 0) {
		pr_err("open %s error: %s.\n", blkname, strerror(errno));
		return -1;
	}

	ret = ioctl(fd, BLKGETSIZE64, &dev_size);
	if (ret)
		pr_err("ioctl BLKGETSIZE64 error: %s.\n", strerror(errno));

	close(fd);

	dev_size = !ret ? dev_size : -1;

	return dev_size;
}
