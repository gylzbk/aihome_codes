#include <stdio.h>
#include "emmc.h"
#include "block.h"

void *emmc_read(char *blkname, off_t offset, size_t len)
{
	return block_read(blkname, offset, len);
}

int emmc_write(char *blkname, off_t offset, unsigned char *data, size_t len)
{
	return block_write(blkname, offset, data, len);
}

int emmc_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size)
{
	return block_write_file(blkname, file, seek, skip, size);
}

int emmc_earse(char *blkname, off_t offset, size_t len)
{
	printf("%s todo.\n", __func__);
	return 0;
}

size_t emmc_size_get(char *blkname)
{
	return block_size_get(blkname);
}

