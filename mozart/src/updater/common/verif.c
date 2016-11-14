#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "updater_interface.h"
#include "flash.h"
#include "verif.h"
#include "openssl/md5.h"

//XXX: memory leak check is needed!!!!
int verif_file_on_flash(char *file, unsigned int offset)
{
	int i = 0;
	int ret = 0;

	ssize_t nread = 0;
	ssize_t nsize = BUFSIZ;
	ssize_t nbytes = 0;
	unsigned char buf[BUFSIZ] = {};

	struct stat s;

	MD5_CTX ctx;

	char *file_md5 = NULL;
	unsigned char md5[16] = {};
	char md5str[34] = {};

	file_md5 = md5sum(file);
	if (!file_md5) {
		printf("can not parse md5 of %s\n", file);
		ret = -1;
		goto fun_exit;
	}

	if (stat(file, &s)) {
		printf("stat(%s) error: %s\n", file, strerror(errno));
		ret = -1;
		goto fun_exit;
	}

	if (1 != MD5_Init(&ctx)) {
		printf("MD5_Init error.\n");
		ret = -1;
		goto fun_exit;
	}

	while (nbytes < s.st_size) {
		nsize = (s.st_size - nbytes) > BUFSIZ ? BUFSIZ : s.st_size - nbytes;
		nread = flash_read(offset + nbytes, buf, nsize);
		if (nread > 0) {
			nbytes += nread;
			if (1 != MD5_Update(&ctx, buf, nread)) {
				printf("MD5_Update error.\n");
				ret = -1;
				goto fun_exit;
			}
		}
	}

	if (1 != MD5_Final(md5, &ctx)) {
		printf("MD5_Update error.\n");
		ret = -1;
		goto fun_exit;
	}

	for(i = 0; i < 16; ++i)
		sprintf(md5str + i * 2, "%02x", md5[i]);

#if 0
	puts(md5str);
	puts(file_md5);
#endif
	if (!strcmp(md5str, file_md5)) {
		ret = 0;
	}

fun_exit:
	if (file_md5) {
		free(file_md5);
		file_md5 = NULL;
	}

	return ret;
}

char *md5sum(char *file)
{
	char data_buf[1024] = {};
	MD5_CTX ctx;
	int fd = -1;
	int nread = 0;
	int i = 0;
	unsigned char md5[16] = {};

	char *md5str = malloc(64);
	memset(md5str, 0, 64);

	fd = open(file, O_RDONLY);
	if (fd == -1) {
		perror("open");
		free(md5str);
		return NULL;
	}

	if (1 != MD5_Init(&ctx)) {
		printf("MD5_Init error.\n");
		close(fd);
		free(md5str);
		return NULL;
	}

	while ((nread = read(fd, data_buf, sizeof(data_buf)))> 0) {
		if (1 != MD5_Update(&ctx, data_buf, nread)) {
			printf("MD5_Update error.\n");
			close(fd);
			free(md5str);
			return NULL;
		}
	}

	if (1 != MD5_Final(md5, &ctx)) {
		printf("MD5_Update error.\n");
		close(fd);
		free(md5str);
		return NULL;
	}

	for(i = 0; i < 16; ++i)
		sprintf(md5str + i * 2, "%02x", md5[i]);

	return md5str;
}

#if 0
int main(int argc , char *argv[])
{
	int i = 0;
	unsigned char *md5 = NULL;

	if (argc != 2) {
		fprintf(stderr, "Please enter the filename\n");
		return -1;
	}

	md5 = md5sum(argv[1]);

	if (!md5) {
		printf("md5sum error.\n");
		return 0;
	}

	//dump md5sum
	puts(md5);

	free(md5);

	return 0;
}
#endif
