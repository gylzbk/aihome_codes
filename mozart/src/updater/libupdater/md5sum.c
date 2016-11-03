#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "openssl/md5.h"

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

