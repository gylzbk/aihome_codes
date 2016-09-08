/*
	mozart_nvrw.c
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mozart_nvrw.h"

#define DRIVER_NAME "/dev/nv_wr"

struct status_flag_bits {
	unsigned int ota_start:1;		/* start */
	unsigned int load_new_fs:1;		/* new_flag */
	unsigned int update_fs_finish:1;	/* update_kernel_finish */
	unsigned int user_fs_finish:1;		/* update_fs_finish */
};

struct nv_area_wr {
	unsigned int	write_start;
	char		url_1[512];
	char		url_2[512];
	float		current_version;
	float		update_version;
	union {
		unsigned int update_status;
		struct status_flag_bits sfb;
	};
	unsigned int	update_result;
	unsigned int	block_current_count;
	unsigned int	kernel_size;
	unsigned int	update_size;
	unsigned char	resever[32 * 1024 - 1024 - 4 * 10];
	unsigned int	write_count;
	unsigned int	write_end;
};

int nvrw_set_update_flag(float version)
{
	struct nv_area_wr *nv_buf;
	int fd;
	ssize_t ioSize;
	int err = 0;

	nv_buf = malloc(32 * 1024);
	if (!nv_buf) {
		printf("[ERROR] %s. Alloc nv buffer: %s\n", __func__, strerror(errno));
		return -1;
	}

	fd = open(DRIVER_NAME, O_RDWR);
	if (fd < 0) {
		printf("[ERROR] %s. open %s: %s\n", __func__, DRIVER_NAME, strerror(errno));
		err = -1;
		goto err_nv_open;
	}

	lseek(fd, 0, SEEK_SET);

	ioSize = read(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. read nv data: %s\n", __func__, strerror(errno));
		goto err_nv_read;
	}

	nv_buf->update_version = version;
	nv_buf->update_status = 0x3;

	ioSize = write(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. write nv back: %s\n", __func__, strerror(errno));
		err = -1;
	}

err_nv_read:
	close(fd);

err_nv_open:
	free(nv_buf);

	return err;
}

int nvrw_get_update_info(struct nv_info *info)
{
	struct nv_area_wr *nv_buf;
	int fd;
	ssize_t rSize;
	int err = 0;

	nv_buf = malloc(32 * 1024);
	if (!nv_buf) {
		printf("[ERROR] %s. Alloc nv buffer: %s\n", __func__, strerror(errno));
		return -1;
	}

	fd = open(DRIVER_NAME, O_RDWR);
	if (fd < 0) {
		printf("[ERROR] %s. open %s: %s\n", __func__, DRIVER_NAME, strerror(errno));
		err = -1;
		goto err_nv_open;
	}

	lseek(fd, 0, SEEK_SET);

	rSize = read(fd, nv_buf, 32*1024);
	if (rSize < 0) {
		printf("[ERROR] %s. read nv data: %s\n", __func__, strerror(errno));
		err = -1;
	}

	info->update_status	= nv_buf->update_status;
	info->update_result	= nv_buf->update_result;
	info->current_version	= nv_buf->current_version;
	info->update_version	= nv_buf->update_version;

	close(fd);

err_nv_open:
	free(nv_buf);

	return err;

}

int nvrw_clear_update_flag(void)
{
	struct nv_area_wr *nv_buf;
	int fd;
	ssize_t ioSize;
	int err = 0;

	nv_buf = malloc(32 * 1024);
	if (!nv_buf) {
		printf("[ERROR] %s. Alloc nv buffer: %s\n", __func__, strerror(errno));
		return -1;
	}

	fd = open(DRIVER_NAME, O_RDWR);
	if (fd < 0) {
		printf("[ERROR] %s. open %s: %s\n", __func__, DRIVER_NAME, strerror(errno));
		err = -1;
		goto err_nv_open;
	}

	lseek(fd, 0, SEEK_SET);

	ioSize = read(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. read nv data: %s\n", __func__, strerror(errno));
		goto err_nv_read;
	}

	nv_buf->current_version = nv_buf->update_version;
	nv_buf->update_version = 0; /* Clear update version */
	nv_buf->update_status = 0; /* Clear update flags */
	nv_buf->update_result = 1;

	ioSize = write(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. write nv back: %s\n", __func__, strerror(errno));
		err = -1;
	}

err_nv_read:
	close(fd);

err_nv_open:
	free(nv_buf);

	return err;
}

int nvrw_clear_update_result(void)
{
	struct nv_area_wr *nv_buf;
	int fd;
	ssize_t ioSize;
	int err = 0;

	nv_buf = malloc(32 * 1024);
	if (!nv_buf) {
		printf("[ERROR] %s. Alloc nv buffer: %s\n", __func__, strerror(errno));
		return -1;
	}

	fd = open(DRIVER_NAME, O_RDWR);
	if (fd < 0) {
		printf("[ERROR] %s. open %s: %s\n", __func__, DRIVER_NAME, strerror(errno));
		err = -1;
		goto err_nv_open;
	}

	lseek(fd, 0, SEEK_SET);

	ioSize = read(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. read nv data: %s\n", __func__, strerror(errno));
		goto err_nv_read;
	}

	nv_buf->update_result = 0; /* Clear update result flag */

	ioSize = write(fd, nv_buf, 32*1024);
	if (ioSize < 0) {
		printf("[ERROR] %s. write nv back: %s\n", __func__, strerror(errno));
		err = -1;
	}

err_nv_read:
	close(fd);

err_nv_open:
	free(nv_buf);

	return err;
}
