#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nvrw_interface.h"
#include "ini_interface.h"

#define COLOR_DEBUG "\033[40;37m"
#define COLOR_ERR "\033[41;37m"
#define COLOR_WARN "\033[43;30m"
#define COLOR_INFO "\033[44;37m"
#define COLOR_CLOSE "\033[0m"

#define NVGEN_DEBUG 0

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



void usage(char *name)
{
	pr_info("%s - a tool to generate nvimage.\n\n"
		   "Usage: %s -c configfile -o nvimage\n\n"
		   "Options:\n"
		   "  -c nvimage config file.\n"
		   "  -o nvimage output file.\n"
		   "  -p nvimage padding size.\n"
		   "\nExample:\n"
		   "  %s -c nvimage.ini -o nv.img -p 0x800.\n"
		   , name, name, name);

	return;
}

struct version {
	int major;
	int minor;
	int revision;
};

static struct version parse_version_str(char *version_str)
{
	char *pos = version_str;
	struct version v = {};

	v.major = atoi(pos);

	pos = strchr(version_str, '.');
	v.minor = atoi(pos + 1);

	pos = strrchr(version_str, '.');
	v.revision = atoi(pos + 1);

	return v;
}

static bool has_new_version(struct nv_info *info)
{
	struct version c_v = parse_version_str(info->current_version);
	struct version u_v = parse_version_str(info->update_version);

	if (u_v.major > c_v.major) {
		return true;
	} else if (u_v.major == c_v.major) {
		if (u_v.minor > c_v.minor) {
			return true;
		} else if (u_v.minor == c_v.minor) {
			if (u_v.revision > c_v.revision) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
		return false;
	}

	return false;
}

int fill_info(nvinfo_t *info, char *config)
{
	int ret = -1;
	char buf[32] = {};

	if (!config || !info)
		return -1;

	if (access(config, 0)) {
		pr_err("%s: Not Found.\n", config);
		return -1;
	}

	strcpy(info->magic, "OTA");

	// used for nvinfo_t->current_version
	ret = mozart_ini_getkey(config, "ota", "current_version", buf);
	if (ret) {
		pr_warn("parse current_version error, force to 1.0.0\n");
		strcpy(info->current_version, "1.0.0");
	} else {
		strcpy(info->current_version, buf);
	}
	pr_debug("info->current_version: v%s.\n", info->current_version);

	// used for nvinfo_t->update_version
	ret = mozart_ini_getkey(config, "ota", "update_version", buf);
	if (ret) {
		strcpy(info->update_version, info->current_version);
	} else {
		if (buf[0] == '\0')
			strcpy(info->update_version, info->current_version);
		else
			strcpy(info->update_version, buf);
	}
	pr_debug("info->update_version: v%s.\n", info->update_version);

	if (has_new_version(info))
		info->update_flag = FLAG_UPDATE;
	else
		info->update_flag = FLAG_NONUPDATE;
	pr_debug("info->update_flag: %x.\n", info->update_flag);

	// used for nvinfo_t->update_method.method
	ret = mozart_ini_getkey(config, "ota", "method", buf);
	if (ret) {
		pr_warn("parse update method error.\n");
		return -1;
	} else {
		if (!strcasecmp("update_once", buf)) {
			info->update_method.method = UPDATE_ONCE;
		} else if (!strcasecmp("update_times", buf)) {
			info->update_method.method = UPDATE_TIMES;
		} else {
			pr_err("Invalid update method: %s, "
				   "only %s/%s(case-insensitive) support now!!\n",
				   buf, "update_once", "update_block");
			return -1;
		}
	}
	pr_debug("info->update_method.method: %d.\n", info->update_method.method);

	// used for nvinfo_t->update_method.storage
	ret = mozart_ini_getkey(config, "ota", "storage",
							(char *)info->update_method.storage);
	if (ret) {
		pr_warn("parse update_method.storage error.\n");
		return -1;
	}
	pr_debug("info->update_method.storage: %s.\n", info->update_method.storage);

	// used for nvinfo_t->update_method.location
	ret = mozart_ini_getkey(config, "ota", "location",
							(char *)info->update_method.location);
	if (ret) {
		pr_warn("parse update_method.location error.\n");
		return -1;
	}
	pr_debug("info->update_method.location: %s.\n", info->update_method.location);

	// used for nvinfo_t->product
	ret = mozart_ini_getkey(config, "ota", "product",
							(char *)info->product);
	if (ret) {
		pr_warn("parse product error.\n");
		return -1;
	}
	pr_debug("info->product: %s.\n", info->product);

	// used for nvinfo_t->url
	ret = mozart_ini_getkey(config, "ota", "url",
							(char *)info->url);
	if (ret) {
		pr_warn("parse url error.\n");
		return -1;
	}
	pr_debug("info->url: %s.\n", info->url);

	return 0;
}

int main(int argc, char **argv)
{
	int c;
	int ret = -1;
	char *config = NULL;
	char *target = NULL;
	nvinfo_t info;
	int fd = -1;
	long int padsize = 0;
	char *padvalue = NULL;

	while (1) {
		c = getopt(argc, argv, "c:o:p:h");
		if (c < 0)
			break;
		switch (c) {
		case 'c':
			config = optarg;
			break;
		case 'o':
			target = optarg;
			break;
		case 'p':
			if (strncmp(optarg, "0x", 2))
				padsize = atoi(optarg);
			else
				padsize = strtol(optarg, NULL, 16);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!config) {
		pr_err("No config file specfied, exit...\n");
		exit(-1);
	}

	if (!target) {
		pr_warn("No output file specfied, use default: nv.img.\n");
		target = "nv.img";
	}

	if (padsize == 0 || sizeof(info) > padsize) {
		pr_err("Wrong padsize.\n");
		return -1;
	}

	memset(&info, 0, sizeof(info));

	ret = fill_info(&info, config);
	if (ret) {
		pr_err("fill info error.\n");
		return -1;
	}

	if (!access(target, 0)) {
		pr_info("%s exist, Remove it? <Y/n> ", target);
		fflush(stdout);

		c = getchar();
		if (c == 'Y' || c == 'y' || c == '\n') {
			pr_warn("%s removed!\n", target);
			unlink(target);
		} else if (c == 'N' || c == 'n') {
			pr_err("Please backup %s firstly!\n", target);
			return 0;
		} else {
			pr_err("Invalid choice!\n");
			return -1;
		}
	}

	fd = open(target, O_CREAT | O_WRONLY, 0666);
	if (fd < 0) {
		pr_err("Canot create %s: %s.\n", target, strerror(errno));
		return -1;
	}

	ret = write(fd, &info, sizeof(info));
	if (ret != sizeof(info)) {
		pr_err("write info to %s error: %s, please re-try!\n",
			   target, strerror(errno));
		goto write_fail;
	}

	padsize -= sizeof(info);
	if (padsize > 0) {
		padvalue = malloc(padsize);
		memset(padvalue, 0, padsize);
		ret = write(fd, padvalue, padsize);
		if (ret != padsize) {
			free(padvalue);
			pr_err("pad %s error: %s, please re-try!\n",
				   target, strerror(errno));
			goto write_fail;
		}
	}

	close(fd);

	return 0;

write_fail:
	if (fd >= 0)
		close(fd);
	unlink(target);

	return -1;
}
