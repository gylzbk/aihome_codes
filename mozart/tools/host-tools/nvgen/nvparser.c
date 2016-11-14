#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nvrw_interface.h"
#include "ini_interface.h"

#define COLOR_DEBUG "\033[40;37m"
#define COLOR_ERR "\033[41;37m"
#define COLOR_WARN "\033[43;30m"
#define COLOR_INFO "\033[44;37m"
#define COLOR_CLOSE "\033[0m"

#define NVGEN_DEBUG 1

#ifdef NVGEN_DEBUG
#define pr_debug(fmt, args...) \
	do { \
		printf(COLOR_DEBUG"%s:%s:%d [Debug]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)
#else
#define pr_debug(fmt, args...)
#endif

#define pr_info(fmt, args...) \
	do { \
		printf(COLOR_INFO"%s:%s:%d [Info]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)

#define pr_warn(fmt, args...) \
	do { \
		printf(COLOR_WARN"%s:%s:%d [Warn]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)

#define pr_err(fmt, args...) \
	do { \
		printf(COLOR_ERR"%s:%s:%d [Error]"COLOR_CLOSE" ", __FILE__, __func__, __LINE__); \
		printf(fmt, ##args);\
	} while (0)



static void usage(char *name)
{
	pr_info("%s - a tool to parse nvimage( and/or export to configfile).\n\n"
		   "Usage: %s -i nvimage [-o configfile]\n\n"
		   "Options:\n"
		   "  -i nvimage nvfile.\n"
		   "  -o nvimage config nvfile.\n"
		   , name, name);

	return;
}

static int export_info_to_file(char *config, nvinfo_t *info)
{
	//current_version
	mozart_ini_setkey(config, "ota", "current_version", info->current_version);

	//update_version
	mozart_ini_setkey(config, "ota", "update_version", info->update_version);

	//method
	if (info->update_method.method == 1)
		mozart_ini_setkey(config, "ota", "method", "UPDATE_ONCE");
	else if (info->update_method.method == 2)
		mozart_ini_setkey(config, "ota", "method", "UPDATE_TIMES");
	else
		mozart_ini_setkey(config, "ota", "method", "UnKnown");

	//storage
	mozart_ini_setkey(config, "ota", "storage", (char *)info->update_method.storage);

	//location
	mozart_ini_setkey(config, "ota", "location", (char *)info->update_method.location);

	//product
	mozart_ini_setkey(config, "ota", "product", (char *)info->product);

	//url
	mozart_ini_setkey(config, "ota", "url", (char *)info->url);

	return 0;
}

static int export_info_to_stdout(nvinfo_t *info)
{
	char *method = NULL;

	if (info->update_method.method == UPDATE_ONCE)
		method = "update_once";
	else if (info->update_method.method == UPDATE_TIMES)
		method = "update_block";
	else
		method = "Unknown";

	printf("[nvimage]\n"
		"current_version = v%s\n"
		"update_version = v%s\n"
		"update_flag = %x\n"
		"method = %s\n"
		"storage = %s\n"
		"location = %s\n"
		"product = %s\n"
		"url = %s\n",
		info->current_version,
		info->update_version,
		info->update_flag,
		method,
		info->update_method.storage,
		info->update_method.location,
		info->product,
		info->url);

	return 0;
}

static int export_info(char *config, nvinfo_t *info)
{
	if (!info || strcmp(info->magic, "OTA")) {
		pr_err("Bad nvimg.\n");
		return -1;
	}

	if (config)
		return export_info_to_file(config, info);
	else
		return export_info_to_stdout(info);
}

int main(int argc, char **argv)
{
	int c;
	int ret = -1;
	char *config = NULL;
	char *nvfile = NULL;
	nvinfo_t info;
	int fd = -1;
	struct stat s;

	while (1) {
		c = getopt(argc, argv, "i:o:h");
		if (c < 0)
			break;
		switch (c) {
		case 'i':
			nvfile = optarg;
			break;
		case 'o':
			config = optarg;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!nvfile) {
		pr_err("No nvimage nvfile specfied.\n");
		return -1;
	}

	memset(&info, 0, sizeof(info));

	if (access(nvfile, 0)) {
		pr_err("%s Not Found.\n", nvfile);
		return -1;
	}

	fd = open(nvfile, O_RDONLY);
	if (fd < 0) {
		pr_err("Canot open %s: %s.\n", nvfile, strerror(errno));
		return -1;
	}

	ret = fstat(fd, &s);
	if (!ret) {
		if (s.st_size != sizeof(info)) {
			pr_err("%s seems NOT a valid nvimage file, please chaek it.\n", nvfile);
			return -1;
		}
	} else {
		// Ignore.
	}

	ret = read(fd, &info, sizeof(info));
	close(fd);
	if (ret == -1) {
		pr_err("read info from %s error: %s.\n", nvfile, strerror(errno));
		return -1;
	}

	if (ret != sizeof(info)) {
		pr_err("read info error, please re-try!\n");
		return -1;
	}

	return export_info(config, &info);
}
