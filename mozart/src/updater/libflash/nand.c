#include <stdio.h>
#include "nand.h"

int nand_read(char *blkname, off_t offset, unsigned char *buf, size_t len)
{
	printf("[nand ops Simulator] %s done successfully.\n", __func__);
	return 0;
}

int nand_write(char *blkname, off_t offset, unsigned char *data, size_t len)
{
	printf("[nand ops Simulator] %s done successfully.\n", __func__);
	return 0;
}

int nand_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size)
{
	printf("[nand ops Simulator] %s done successfully.\n", __func__);
	return 0;
}

int nand_earse(char *blkname, off_t offset, size_t len)
{
	printf("[nand ops Simulator] %s done successfully.\n", __func__);
	return 0;
}

size_t nand_size_get(char *blkname)
{
	printf("[nand ops Simulator] %s done successfully.\n", __func__);
	return 0;
}
