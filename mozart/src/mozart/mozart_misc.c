/*
	mozart_misc.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/vfs.h>

#include "utils_interface.h"
#include "mozart_misc.h"

#ifndef MOZART_RELEASE
#define MOZART_MISC_DEBUG
#endif

#ifdef MOZART_MISC_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[MISC] %s: "fmt, __func__, ##args)
#else  /* MOZART_MISC_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_MISC_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[MISC] [Error] %s: "fmt, __func__, ##args)

#define pr_info(fmt, args...)			\
	fprintf(stderr, "[MISC] [Info] %s: "fmt, __func__, ##args)

int mozart_ext_storage_enough_free(const char *path, int reserve_size)
{
	struct statfs fs_s;
	fsblkcnt_t reserve_blocks;
	int err;

	err = statfs(path, &fs_s);
	if (err < 0) {
		pr_err("Get disk stat: %s\n", strerror(errno));
		return false;
	}

	pr_debug("'%s' state: f_bfree: %ld, f_bavail: %ld, f_blocks: %ld, f_bsize: %ld\n",
			path, fs_s.f_bfree, fs_s.f_bavail, fs_s.f_blocks, fs_s.f_bsize);

	reserve_blocks = (reserve_size + (fs_s.f_bsize - 1)) / fs_s.f_bsize;
	pr_info("reserv blocks: %ld\n", reserve_blocks);

	return fs_s.f_bavail >= reserve_blocks;
}

int mozart_ext_storage_file_clear(const char *dir)
{
	DIR *dir_p;
	struct dirent *entry;
	int count = 0;

	dir_p = opendir(dir);
	if (!dir_p) {
		pr_err("Open dir '%s': %s\n", dir, strerror(errno));
		return -1;
	}

	while ((entry = readdir(dir_p)) != NULL) {
		/* Unlink favorite file exclude *.ucache */
		if (!strstr(entry->d_name, ".ucache")) {
			char *target = malloc(strlen(dir) + strlen(entry->d_name) + sizeof("/"));
			if (!target) {
				pr_err("Alloc unlink target: %s\n", strerror(errno));
				closedir(dir_p);
				return -1;
			}

			sprintf(target, "%s/%s", dir, entry->d_name);
			unlink(target);

			free(target);
			count++;
		}
	}

	closedir(dir_p);

	pr_info("Cleared %d favorite files\n", count);

	return 0;
}
