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

#include "debug.h"

#define CHECK_MD5 1

#if CHECK_MD5
#include "openssl/md5.h"
#endif

size_t block_size_get(char *blkname)
{
	int ret = -1;
    int fd = -1;
    long long dev_size = -1;

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

void *block_read(char *blkname, off_t offset, size_t size)
{
	int fd = -1;
	void *data = NULL;
	int blksize = -1;

	if (!blkname || size <= 0)
		return NULL;

	if (offset < 0)
		offset = 0;

	/* check offset + size <= block_size */
	blksize = block_size_get(blkname);
	if ((offset + size) > blksize) {
		pr_err("Invalid value, offset(%lu) + size(%d) > blksize(%d)\n",
			   offset, size, blksize);
		return NULL;
	}

	data = malloc(size);
	if (!data) {
		pr_err("Alloc data buffer: %s\n", strerror(errno));
		return NULL;
	}
	memset(data, 0, size);

    fd = open(blkname, O_RDONLY);
    if (fd < 0) {
		pr_err("open %s error: %s.\n", blkname, strerror(errno));
		goto err_out;
	}

	if (offset != lseek(fd, offset, SEEK_SET)) {
		pr_err("lseek to %lu bytes error: %s.\n", offset, strerror(errno));
		goto err_out;
	}

	if (size != read(fd, data, size)) {
		pr_err("read %d bytes error: %s.\n", size, strerror(errno));
		goto err_out;
	}

	close(fd);

	return data;

err_out:
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}

	free(data);
	data = NULL;

	return NULL;
}

int block_write(char *blkname, off_t offset, void *data, size_t size)
{
	int fd = -1;
	size_t ret = -1;
	size_t blksize = -1;

	if (!blkname || !data || size < 0)
		return -1;

	if (size == 0) {
		pr_warn("write 0 bytes, ignore, but success.\n");
		return 0;
	}

	if (offset < 0)
		offset = 0;

	/* check offset + size <= block_size */
	blksize = block_size_get(blkname);
	if ((offset + size) > blksize) {
		pr_err("Invalid value, offset(%lu) + size(%d) > blksize(%d)\n",
			   offset, size, blksize);
		return -1;
	}

    fd = open(blkname, O_WRONLY);
    if (fd < 0) {
		pr_err("open %s error: %s.\n", blkname, strerror(errno));
		goto err_out;
	}

	if (offset != lseek(fd, offset, SEEK_SET)) {
		pr_err("lseek to %lu bytes error: %s.\n", offset, strerror(errno));
		goto err_out;
	}

	ret = write(fd, data, size);
	if (size != ret) {
		pr_err("write %d(real %d) bytes error: %s.\n",
			   size, ret, strerror(errno));
		goto err_out;
	}

	close(fd);

	return 0;

err_out:
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}

	free(data);
	data = NULL;

	return -1;
}

int block_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size)
{
#define BUFSIZE (128 * 1024)
#define TRY_CNT_MAX 3
	int ret = 0;
	int ifd = -1;
	off_t pos = -1;
	ssize_t nread = 0;
	int try_cnt = 0;
	char data[BUFSIZE] = {};
	int read_size = 0;
	int  read_cnt = 0;

#if CHECK_MD5
	int n = 0;
	MD5_CTX ctx;
	MD5_CTX f_ctx;
	unsigned char md5[16] = {};
	char *md5str= NULL;
	unsigned char f_md5[16] = {};
	char *f_md5str= NULL;
	char *read_data = NULL;
#endif

	if (!blkname || !file) {
		pr_err("invalid arg, blkname: %s, file: %s.\n",
			   blkname, file);
		return -1;
	}

	if (seek > 0) {
		if (seek >= block_size_get(blkname)) {
			pr_err("seek >= block_size_get(blkname).\n");
			return -1;
		}
	}

#if CHECK_MD5
	MD5_Init(&ctx);
	MD5_Init(&f_ctx);
#endif

	ifd = open(file, O_RDONLY);
	if (ifd < 0) {
		pr_err("open %s: %s.\n", file, strerror(errno));
		return -1;
	}

	if (skip > 0) {
		if (skip != lseek(ifd, skip, SEEK_SET)) {
			pr_err("lseek to %ld bytes error: %s.\n", skip, strerror(errno));
			return -1;
		}
	}

	pos = seek;
	read_size = read_size > size ? size : BUFSIZE;
	read_cnt = 0;
	while (read_cnt < size) {
		if ((read_cnt + read_size) > size)
			read_size = size - read_cnt;
		nread = read(ifd, data, read_size);
#if CHECK_MD5
		MD5_Update(&f_ctx, data, nread);
#endif
		read_cnt += nread;
		printf("\rprocess: %#05x/%#04x.", read_cnt, size);
		fflush(stdout);
block_write_retry:
		if (!block_write(blkname, pos, data, nread)) {
#if CHECK_MD5
			read_data = block_read(blkname, pos, nread);
			MD5_Update(&ctx, read_data, nread);
			free(read_data);
#endif
			pos += nread;
		} else {
			if (++try_cnt >= TRY_CNT_MAX) {
				pr_err("block write fail.\n");
				return -1;
			}
			usleep(10 * 1000);
			goto block_write_retry;
		}
	}
	puts("\n");

#if CHECK_MD5
	MD5_Final(md5, &ctx);
	md5str= malloc(64);
	memset(md5str, 0, 64);

	MD5_Final(f_md5, &f_ctx);
	f_md5str= malloc(64);
	memset(f_md5str, 0, 64);

	for (n = 0; n < 16; ++n) {
		sprintf(md5str + n * 2, "%02x", md5[n]);
		sprintf(f_md5str + n * 2, "%02x", f_md5[n]);
	}

	if (!strcmp(md5str, f_md5str)) {
		pr_info("check flash file md5 pass.\n");
		ret = 0;
	} else {
		pr_err("flash file md5 check fail.\n");
		ret = -1;
	}
	free(md5str);
	free(f_md5str);
#endif

	return ret;
}
