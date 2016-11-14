#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "ini_interface.h"
#include "nvrw_interface.h"
#include "utils_interface.h"
#include "flash.h"
#include "block.h"
#include "debug.h"
#include "downloader.h"
#include "md5sum.h"

int mozart_updater_nvrw(nvinfo_t *nvinfo)
{
	int nvrw_lock = 0;

	nvrw_lock = mozart_nv_lock();
	if (mozart_nv_set_nvinfo(nvinfo) != 0) {
		mozart_nv_unlock(nvrw_lock);
		return -1;
	}
	mozart_nv_unlock(nvrw_lock);
	return 0;
}

static int update_info_getversion(char *infofile, char *version_out)
{
	if (mozart_ini_getkey(infofile, "update", "version", version_out))
		return -1;

	return 0;
}

static int update_info_getfilelist(nvinfo_t *info, char *o_file)
{
	int ret = 0;
	char *i_file = NULL;

	i_file = malloc(128);
	if (!i_file) {
		pr_err("alloc for filelist file fail: %s.\n", strerror(errno));
		return -1;
	}
	sprintf(i_file, "v%s/%s", info->update_version, "filelist.ini");

	ret = update_remote_get(info, i_file, o_file);

	if (ret) {
		pr_err("can not get versionfile: %s.\n", i_file);
		ret = -1;
	}

	free(i_file);

	return ret;
}

static int update_info_parse_updatefiles(char *versionfile, struct nv_info *info)
{
	int i = 0;
	int offset = 0;
	int cnt = UPDATE_FILE_MAX;
	char **filenames = NULL;
	char size_buf[16] = {};

	filenames = mozart_ini_getsections1(versionfile, &cnt);
	if (!filenames) {
		pr_err("parse %s error.\n", versionfile);
		return -1;
	}

	info->otafile_cnt = cnt;

	for (i = 0; i < cnt; i++) {
		strncpy(info->files[i].name, filenames[i], sizeof(info->files[i].name) - 1);
		mozart_ini_getkey(versionfile, filenames[i], "md5", info->files[i].md5);
		mozart_ini_getkey(versionfile, filenames[i], "target", info->files[i].blkdev);
		mozart_ini_getkey(versionfile, filenames[i], "size", size_buf);
		info->files[i].size = atoi(size_buf);
		info->files[i].offset = offset;
		strcpy((char *)info->files[i].location, (char *)info->update_method.location);
		pr_debug("%s will be at %#llx.\n", info->files[i].name, info->files[i].offset);
		offset += block_size_get(info->files[i].blkdev);
		free(filenames[i]);
	}

	free(filenames);

	return 0;
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

struct nv_info *mozart_updater_chkver(void)
{
	int ret = 0;
	int nvrw_lock = -1;
	struct nv_info *info = NULL;

	// get nvinfo
	nvrw_lock = mozart_nv_lock();
	info = mozart_nv_get_nvinfo();
	if (!info) {
		mozart_nv_unlock(nvrw_lock);
		return NULL;
	}
	mozart_nv_unlock(nvrw_lock);

	// get update version
	ret = update_remote_get(info, "info.ini", "/tmp/info.ini");
	if (ret) {
		pr_err("get update info fail.\n");
		goto err_out;
	}
	ret = update_info_getversion("/tmp/info.ini", info->update_version);
	if (ret) {
		pr_debug("get update version fail.\n");
		goto err_out;
	}

	// check version
	// 1. have new version, return new version.
	// 2. no new version, return 0.0
	if (!has_new_version(info)) {
		pr_debug("no new version found(curr: v%s, update: v%s).\n",
				info->current_version, info->update_version);
		goto err_out;
	}

	// parse update files.
	//1. download info file about new version
	ret = update_info_getfilelist(info, "/tmp/filelist.ini");
	if (ret) {
		pr_err("download filelist.ini fail.\n");
		goto err_out;
	}

	//2. parse update file list.
	ret = update_info_parse_updatefiles("/tmp/filelist.ini", info);
	if (ret) {
		pr_err("parse update files fail.\n");
		goto err_out;
	}

	return info;

err_out:
	system("rm -f /tmp/info.ini");
	system("rm -f /tmp/filelist.ini");
	if (info) {
		free(info);
		info = NULL;
	}

	return info;
}

static struct otafile_info get_otafile_info(struct nv_info *info, char *filename)
{
	int i = 0;
	struct otafile_info f;

	memset(&f, 0, sizeof(f));

	for (i = 0; i < info->otafile_cnt; i++) {
		if (!strcmp(info->files[i].name, filename)) {
			memcpy(&f, &info->files[i], sizeof(f));
			break;
		}
	}

	return f;
}

static int update_info_download_updatefile(struct nv_info *info, char *filename)
{
	int ret = 0;
	int url_len = 0;
	char i_file[128] = {};
	char *url = NULL;
	struct otafile_info f;

	f = get_otafile_info(info, filename);

	if (!strlen(f.name)) {
		pr_err("%s's info not found.\n", filename);
		return -1;
	}

	sprintf(i_file, "v%s/%s", info->update_version, f.name);

	url_len = strlen((char *)info->url) +
		strlen((char *)info->product) + strlen(i_file) + 3;
	url = malloc(url_len);
	snprintf(url, url_len, "%s/%s/%s",
			 (char *)info->url, (char *)info->product, i_file);

	pr_info("download %s to %s +%#llx\n",
			i_file, (char *)f.location, f.offset);
	ret = mozart_updater_download_to_flash(url, f);
	free(url);

	if (ret) {
		pr_err("download %s error.\n", i_file);
		return -1;
	}

	return ret;
}


static char *get_mtd(char *mtdblock)
{
	char mtd[] = "/dev/mtdX";

	if (!mtdblock)
		return NULL;

	/* not mtd */
	if (strncmp(mtdblock, "/dev/mtdblock", strlen("/dev/mtdblock")))
		return NULL;

	sprintf(mtd, "/dev/mtd%c", mtdblock[strlen(mtdblock) - 1]);

	return strdup(mtd);
}

static int erase_blk(char *blkname)
{
	char *mtd = NULL;
	char cmd[128] = {};

	/* erase before write */
	mtd = get_mtd(blkname);
	if (mtd) {
		pr_info("erasing %s.\n", blkname);
		sprintf(cmd, "flash_erase %s 0 0", mtd);
		free(mtd);
		system(cmd);
	}

	return 0;
}

static int download_and_updatefile(struct nv_info *info, char *filename)
{
	int i = 0;
	int ret = -1;

	for (i = 0; i < info->otafile_cnt; i++) {
		if (!strcmp(info->files[i].name, filename)) {
			erase_blk((char *)info->files[i].blkdev);
			strcpy((char *)info->files[i].location,
					(char *)info->files[i].blkdev);
			info->files[i].offset = 0;
			ret = update_info_download_updatefile(info, filename);
			if (ret)
				return ret;
			system("sync");
			return ret;
		}
	}

	return ret;
}

static int update_info_download_updatefiles(struct nv_info *info)
{
	int i = 0;
	int ret = 0;

	if (info->update_method.method == UPDATE_ONCE) {
		if (info->update_process == PROCESS_1) {
			erase_blk((char *)info->update_method.location);
			ret = update_info_download_updatefile(info, "updater");
			if (ret)
				return -1;
			system("sync");
			ret = update_info_download_updatefile(info, "kernel");
			if (ret)
				return -1;
			system("sync");
		} else if (info->update_process == PROCESS_2) {
			ret = download_and_updatefile(info, "kernel");
			if (ret)
				return -1;
			ret = download_and_updatefile(info, "updater");
			if (ret)
				return -1;
			ret = download_and_updatefile(info, "appfs");
			if (ret)
				return -1;
		}
	} else if (info->update_method.method == UPDATE_TIMES) {
		if (info->update_process == PROCESS_1) {
			erase_blk((char *)info->update_method.location);
			ret = update_info_download_updatefile(info, "updater");
			if (ret)
				return -1;
			system("sync");
			ret = update_info_download_updatefile(info, "kernel");
			if (ret)
				return -1;
			system("sync");
		} else if (info->update_process == PROCESS_3) {
			erase_blk((char *)info->update_method.location);
			/* fix appfs offset to 0 */
			for (i = 0; i < info->otafile_cnt; i++) {
				if (!strcmp(info->files[i].name, "appfs")) {
					info->files[i].offset = 0;
					break;
				}
			}
			ret = update_info_download_updatefile(info, "appfs");
			system("sync");
		}
	}

	return ret;
}

int mozart_updater_download(struct nv_info *info)
{
	int ret = 0;

	//3. download update files.
	ret = update_info_download_updatefiles(info);
	if (ret) {
		pr_err("download update files fail.\n");
		return -1;
	}

	return 0;
}

static int mozart_updater_updatefile(struct nv_info *info, char *filename)
{
#define TRY_CNT_MAX 3
	int try_cnt = 0;
	int ret = 0;
	char file_path[64] = {};

	struct otafile_info f;

	f = get_otafile_info(info, filename);
	erase_blk((char *)f.blkdev);

	sprintf(file_path, "%s/%s", info->update_method.location, f.name);

	// FIXME; if write error.
flash_write_retry:
	// FIXME: erase here first.
	pr_info("writing %s to %s.\n", f.name, f.blkdev);
	ret = flash_write_file(f.blkdev, (char *)info->update_method.location, 0, f.offset, f.size);
	if (ret) {
		if (++try_cnt >= TRY_CNT_MAX) {
			pr_err("write %s to %s fail.\n", f.name, f.blkdev);
			return -1;
		}
		usleep(10 * 1000);
		goto flash_write_retry;
	}

	return 0;
}

int mozart_updater_update(struct nv_info *info)
{
	int ret = 0;
	int i = 0;

	if (info->update_method.method == UPDATE_ONCE) {
		for (i = 0; i < info->otafile_cnt; i++) {
			ret = mozart_updater_updatefile(info, info->files[i].name);
			if (ret)
				return -1;
			system("sync");
		}
	} else if (info->update_method.method == UPDATE_TIMES) {
		if (info->update_process == PROCESS_2) {
			ret = mozart_updater_updatefile(info, "updater");
			if (ret)
				return -1;
			system("sync");
			ret = mozart_updater_updatefile(info, "kernel");
			if (ret)
				return -1;
			system("sync");
		}
	}

	return ret;
}
