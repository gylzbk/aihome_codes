/*
	mozart_update.c
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <openssl/md5.h>

#include "utils_interface.h"
#include "ini_interface.h"

#include "mozart_nvrw.h"
#include "mozart_update.h"

struct package_info {
	char		md5Str[MD5_DIGEST_LENGTH * 2 + 1];
	char		pack[128];
	float		ver;
};

struct update_t {
	pthread_t	pthread;

	int		msg_pipe[2];
	long		interval_time; /* Unit: second */

	float		update_ver;

	/* Newer version found callbak */
	ResultCallBack_t (*update_new_callback)(float version, void *priv);
	void *priv_data;
};

static struct update_t mozart_update = {
	.msg_pipe		= {-1, -1},
	.update_ver		= 0.0,
	.update_new_callback	= NULL,
};

static char *infoFile = "/tmp/update-info.ini";
static char *iniFile = "/usr/data/system.ini";
static char *loPackPath = "/mnt/sdcard";
static char otaUrl[512];

static int update_remote_get(char *r_path, char *lo_path)
{
	char *exep;
	char timeout_str[11];
	int tout = 30; /* Default 30s */
	int err;

	err = mozart_ini_getkey(iniFile, "Update", "remote-url", otaUrl);
	if (err) {
		printf("[ERROR] %s. Not get remote-url\n", __func__);
		return -1;
	}

	err = mozart_ini_getkey(iniFile, "Update", "timeout", timeout_str);
	if (!err)
		tout = atoi(timeout_str);

	exep = malloc(strlen(otaUrl) + strlen(r_path) + sizeof("/") + 128);
	if (!exep) {
		printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
		return -1;
	}

	unlink(lo_path);

	/* Download update version info file */
	sprintf(exep, "wget -O %s -T %d '%s/%s'", lo_path, tout, otaUrl, r_path);
	err = mozart_system(exep);
	free(exep);

	if (err == -1) {
		printf("[ERROR] %s. Run wget failed\n", __func__);
		return -1;
	}

	return WEXITSTATUS(err);
}

static int update_info_get(char *target)
{
	return update_remote_get("info.ini", target);
}

static int update_info_parse_version(char *file, float *version)
{
	char version_str[32];

	if (mozart_ini_getkey(file, "UpdateInfo", "version", version_str))
		return -1;

	*version = strtof(version_str + 1, NULL);

	return 0;
}

static int update_info_parse_total(char *file, int *total_size)
{
	char total_size_str[32];
	int err;

	err = mozart_ini_getkey(file, "UpdateInfo", "size", total_size_str);

	/* total_size unit: kBytes */
	*total_size = err ? 0 : atoi(total_size_str);

	return err ? -1 : 0;
}

static int update_check_version(float remote, float current)
{
	return remote > current;
}

static void *update_check_func(void *data)
{
	pthread_detach(pthread_self());
	struct nv_info info;
	float current_ver, new_ver;
	int retval;
	int err;

	fd_set fds;
	struct timeval tv = {
		.tv_sec		= 0,
		.tv_usec	= 0,
	};

	/* Get local version */
	err = nvrw_get_update_info(&info);
	if (err < 0) {
		printf("[ERROR] %s. mozart Get nv info failed\n", __func__);
		return NULL;
	}

	current_ver = info.current_version;

	printf("[Info] %s, Mozart version: v%.5f\n", __func__, current_ver);

	for (;;) {
		/* Clear select fds */
		FD_ZERO(&fds);
		/* Set pipe read end */
		FD_SET(mozart_update.msg_pipe[0], &fds);

		retval = select(mozart_update.msg_pipe[0] + 1, &fds, NULL, NULL, &tv);
		if (retval < 0) {
			printf("[ERROR] %s. select: %s\n", __func__, strerror(errno));
			goto err_select;
		}

		if (!retval) {
			err = update_info_get(infoFile);
			if (err < 0) {
				goto err_get_info;
			} else if (err) {
				printf("[WARNING] %s. Get info: %s\n", __func__, strerror(err));
				goto abort_this_time;
			}

			/* Parse version */
			if (update_info_parse_version(infoFile, &new_ver)) {
				printf("[WARNING] %s. No version info found\n", __func__);
				/* No version info found */
				goto abort_this_time;
			}

			/* Compare remote version and current version */
			if (update_check_version(new_ver, current_ver)) {
				printf("[Attention] %s. Newer version found.\n", __func__);
				if (mozart_update.update_new_callback) {
					err = mozart_update.update_new_callback(new_ver, mozart_update.priv_data);
					switch (err) {
					case UNRECOVERABLE_ERR:
						printf("[Error] %s. update_new_callback unrecoverable error.\n", __func__);
						goto err_callback_unrecoverable;

					case NEW_VERSION_ABORT:
						printf("[WARNING] %s. New version abort.\n", __func__);
						goto abort_this_time;

					case NEW_VERSION_OK:
						break;
					}
				}

				break;
			}

abort_this_time:
			tv.tv_sec = mozart_update.interval_time;
			continue;
		} else {
			if (FD_ISSET(mozart_update.msg_pipe[0], &fds)) {
				char msg = 0;
				read(mozart_update.msg_pipe[0], &msg, sizeof(msg));
				/* Quit update check */
				break;
			}
			tv.tv_sec = mozart_update.interval_time;
		}
	}

err_callback_unrecoverable:
err_get_info:
err_select:
	return NULL;
}

int mozart_update_check_start(
	ResultCallBack_t (*newer_callback)(float, void *),
	void *priv,
	long interval_time)
{
	int err;

	if (mozart_update.msg_pipe[0] >= 0 ||
	    mozart_update.msg_pipe[1] >= 0) {
		printf("[ERROR] %s. update check thread has running\n", __func__);
		return -1;
	}

	mozart_update.update_new_callback	= newer_callback;
	mozart_update.priv_data			= priv;
	mozart_update.interval_time		= interval_time;

	err = pipe(mozart_update.msg_pipe);
	if (err < 0) {
		printf("[ERROR] %s. Get msg pipe: %s\n", __func__, strerror(errno));
		return -1;
	}

	err = pthread_create(&mozart_update.pthread, NULL, update_check_func, NULL);
	if (err) {
		printf("[ERROR] %s. Create update check thread: %s\n", __func__, strerror(err));
		/* Close pipe */
		close(mozart_update.msg_pipe[0]);
		mozart_update.msg_pipe[0] = -1;
		close(mozart_update.msg_pipe[1]);
		mozart_update.msg_pipe[1] = -1;
		return -1;
	}

	return 0;
}

int mozart_update_check_stop(void)
{
	char msg = 'Q';

	if (mozart_update.msg_pipe[0] < 0 ||
	    mozart_update.msg_pipe[1] < 0) {
		printf("[WRANING] %s. update check thread not running\n", __func__);
		return -1;
	}

	/* Send poll quit message to mdns pthread */
	write(mozart_update.msg_pipe[1], &msg, sizeof(msg));
	mozart_system("killall wget");
	/* Wait pthread stoped */
	pthread_join(mozart_update.pthread, NULL);

	mozart_update.update_new_callback = NULL;

	/* Close pipe */
	close(mozart_update.msg_pipe[0]);
	mozart_update.msg_pipe[0] = -1;
	close(mozart_update.msg_pipe[1]);
	mozart_update.msg_pipe[1] = -1;

	return 0;
}

static int update_get_package_info(struct package_info *info)
{
	char *pack_info, *tmp_file;
	char *line = NULL;
	size_t len = 0;
	ssize_t rSize;
	FILE *fp;
	int err = -0;

	pack_info = malloc(sizeof("update_v/info_v") + 64);
	if (!pack_info) {
		printf("[ERROR] %s. Alloc pack_info name: %s\n", __func__, strerror(errno));
		return -1;
	}
	sprintf(pack_info, "update_v%.5f/info_v%.5f", info->ver, info->ver);

	tmp_file = malloc(sizeof("/tmp/update-info_") + 32);
	if (!tmp_file) {
		printf("[ERROR] %s. Alloc pack_info name: %s\n", __func__, strerror(errno));
		err = -1;
		goto err_alloc_tmpfile;
	}
	sprintf(tmp_file, "/tmp/update-info_v%.5f", info->ver);

	err = update_remote_get(pack_info, tmp_file);
	if (err) {
		if (err < 0)
			printf("[ERROR] %s. Run Wget failed\n", __func__);
		else
			printf("[ERROR] %s. Get package info: %s\n", __func__, strerror(err));

		err = -1;
		goto err_get_pack_info;
	}

	fp = fopen(tmp_file, "rb");
	if (!fp) {
		printf("[ERROR] %s. fopen pack info: %s\n", __func__, strerror(errno));
		err = -1;
		goto err_open;
	}

	while ((rSize = getline(&line, &len, fp)) != -1) {
		if (rSize) {
			char *name;
			/* Remove '\n' */
			line[rSize - 1] = '\0';

			strncpy(info->md5Str, line, MD5_DIGEST_LENGTH * 2);
			info->md5Str[MD5_DIGEST_LENGTH * 2] = '\0';

			name = strrchr(line, ' ');
			if (!name) {
				printf("[ERROR] %s. not found file name\n", __func__);
				fclose(fp);
				goto err_strrchr;
			}
			strcpy(info->pack, name + 1);

			printf("%s. Remote MD5 value: %s\n", __func__, info->md5Str);
			break;
		}
	}

	if (line)
		free(line);

	fclose(fp);

	if (rSize < 0) {
		printf("[ERROR] %s. info File is NULL\n", __func__);
		err = -1;
	}

err_strrchr:
err_open:
err_get_pack_info:
	free(tmp_file);

err_alloc_tmpfile:
	free(pack_info);

	return err;
}

static int update_md5_check(char *file, char *md5_val)
{
	MD5_CTX ctx;
	char buf[512];
	uint8_t md5sum[MD5_DIGEST_LENGTH];
	char md5_str[MD5_DIGEST_LENGTH * 2 + 1] = {0};
	ssize_t rSize;
	int i;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("[ERROR] %s. open %s : %s\n", __func__, file, strerror(errno));
		return -1;
	}

	MD5_Init(&ctx);

	do {
		rSize = read(fd, buf, 512);
		if (rSize < 0) {
			printf("[ERROR] %s. read file: %s\n", __func__, strerror(errno));
			MD5_Final(md5sum, &ctx);
			close(fd);
			return -1;
		} else if (rSize) {
			MD5_Update(&ctx, buf, rSize);
		}
	} while (rSize);

	MD5_Final(md5sum, &ctx);

	close(fd);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(md5_str + 2 * i, "%02x", md5sum[i]);

	printf("%s. Download MD5 value: %s\n", __func__, md5_str);

	return strncmp(md5_str, md5_val, MD5_DIGEST_LENGTH);
}

static int update_download_package(struct package_info *info)
{
	char *pack_path, *tmp_file;
	int err = 0;

	pack_path = malloc(sizeof("update_v/") + strlen(info->pack) + 32);
	if (!pack_path) {
		printf("[ERROR] %s. Alloc pack_path name: %s\n", __func__, strerror(errno));
		return -1;
	}
	sprintf(pack_path, "update_v%.5f/%s", info->ver, info->pack);

	tmp_file = malloc(strlen(loPackPath) + sizeof("/") + strlen(info->pack));
	if (!tmp_file) {
		printf("[ERROR] %s. Alloc pack_info name: %s\n", __func__, strerror(errno));
		err = -1;
		goto err_alloc_tmpfile;
	}
	sprintf(tmp_file, "%s/%s", loPackPath, info->pack);

	err = update_remote_get(pack_path, tmp_file);
	if (err) {
		if (err < 0)
			printf("[ERROR] %s. Run Wget failed\n", __func__);
		else
			printf("[ERROR] %s. Get update package: %s\n", __func__, strerror(err));

		err = -1;
		goto err_get_package;
	}

	/* File check */
	err = update_md5_check(tmp_file, info->md5Str);
	if (err) {
		printf("[ERROR] %s. download package is broken\n", __func__);
		err = -1;
	}

err_get_package:
	free(tmp_file);
err_alloc_tmpfile:
	free(pack_path);

	return err;
}

static int update_unpack_package(char *src_file)
{
	char *exep;
	int err;

	exep = malloc(strlen(src_file) + strlen(loPackPath) * 2 + sizeof("unzip -qo / -d "));
	if (!exep) {
		printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
		return -1;
	}
	sprintf(exep, "unzip -qo %s/%s -d %s", loPackPath, src_file, loPackPath);

	err = mozart_system(exep);
	free(exep);
	if (err == -1) {
		printf("[ERROR] %s. Run wget failed\n", __func__);
		return -1;
	} else if (WEXITSTATUS(err)) {
		printf("[ERROR] %s. Run unzip: %s\n", __func__, strerror(WEXITSTATUS(err)));
		return -1;
	}

	return 0;
}

static int update_usrdata_backup(void)
{
	char *exep;
	int err;

	/* Try to clean data target */
	exep = malloc(strlen(loPackPath) + sizeof("rm -rf /data"));
	if (!exep) {
		printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
		return -1;
	}
	sprintf(exep, "rm -rf %s/data", loPackPath);

	err = mozart_system(exep);
	free(exep);
	if (err == -1) {
		printf("[ERROR] %s. Run rm failed\n", __func__);
		return -1;
	} else if (WEXITSTATUS(err)) {
		printf("[ERROR] %s. Run rm: %s\n", __func__, strerror(WEXITSTATUS(err)));
		return -1;
	}

	exep = malloc(strlen(loPackPath) + sizeof("cp -r /usr/data "));
	if (!exep) {
		printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
		return -1;
	}
	sprintf(exep, "cp -r /usr/data %s", loPackPath);

	err = mozart_system(exep);
	free(exep);
	if (err == -1) {
		printf("[ERROR] %s. Run cp failed\n", __func__);
		return -1;
	} else if (WEXITSTATUS(err)) {
		printf("[ERROR] %s. Run cp: %s\n", __func__, strerror(WEXITSTATUS(err)));
		return -1;
	}

	return 0;
}

static void update_clear(struct package_info *info, int deep)
{
	char *path;
	char *exep;

	path = malloc(strlen(info->pack) + strlen(loPackPath) + sizeof("/"));
	if (!path) {
		printf("[ERROR] %s. Alloc path: %s\n", __func__, strerror(errno));
		return;
	}
	sprintf(path, "%s/%s", loPackPath, info->pack);
	unlink(path);
	free(path);

	if (deep) {
		exep = malloc(strlen(loPackPath) + sizeof("rm -rf /update* > /dev/null"));
		if (!exep) {
			printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
			return;
		}
		sprintf(exep, "rm -rf %s/update* > /dev/null", loPackPath);
		mozart_system(exep);
		free(exep);
	}
}

static int update_prepear(
	void (*tsize_callback)(char *, int, void *),
	void *tsize_priv)
{
	struct package_info info;
	float ver;
	int total_size;
	int err;

	/* Stop update check */
	if (mozart_update.msg_pipe[0] >= 0)
		mozart_update_check_stop();

	/* TODO ? switch Network to STA */

	err = update_info_get(infoFile);
	if (err) {
		printf("[ERROR] %s. Get info: %s\n", __func__, strerror(err));
		return -1;
	}

	/* Parse version */
	err = update_info_parse_version(infoFile, &ver);
	if (err < 0) {
		/* No version info found */
		printf("[ERROR] %s. No version info found\n", __func__);
		return -1;
	}

	err = update_info_parse_total(infoFile, &total_size);
	if (err < 0) {
		/* No total size info found */
		printf("[WARNING] %s. No total size info found\n", __func__);
	}

	info.ver			= ver;
	mozart_update.update_ver	= ver;

	err = update_get_package_info(&info);
	if (err < 0)
		return -1;

	if (tsize_callback) {
		char *tmp_file = malloc(strlen(loPackPath) + sizeof("/") + strlen(info.pack));
		if (!tmp_file) {
			printf("[ERROR] %s. Alloc pack_info name: %s\n", __func__, strerror(errno));
			return -1;
		}
		sprintf(tmp_file, "%s/%s", loPackPath, info.pack);

		tsize_callback(tmp_file, total_size, tsize_priv);

		free(tmp_file);
	}

	/* Download file and check correct */
	err = update_download_package(&info);
	if (err < 0) {
		update_clear(&info, 0);
		return -1;
	}

	/* Unpack package */
	err = update_unpack_package(info.pack);
	if (err < 0) {
		update_clear(&info, 1);
		return -1;
	}

	err = update_usrdata_backup();
	if (err < 0) {
		update_clear(&info, 1);
		return -1;
	}

	mozart_system("sync");

	return 0;
}

static int update_active(float version)
{
	/* Set update flag */
	if (nvrw_set_update_flag(version) < 0)
		return -1;

	/* Reboot system */
	mozart_system("reboot -d 4");

	return 0;
}

int mozart_update_start(
	void (*perform_callback)(float, void *),
	void *per_priv,
	void (*tsize_callback)(char *, int, void *),
	void *tsize_priv)
{
	if (update_prepear(tsize_callback, tsize_priv) < 0)
		return -1;

	if (perform_callback)
		perform_callback(mozart_update.update_ver, per_priv);

	return update_active(mozart_update.update_ver);
}

int mozart_update_cancel(void)
{
	return 0;
}

int mozart_direct_update(void)
{
	char ver_str[32];

	if (!mozart_ini_getkey("/mnt/sdcard/update.ini", "Update", "version", ver_str)) {
		printf("[INFO] %s. Go to SDcard update directly\n", __func__);
		update_active(strtof(ver_str, NULL));
	} else {
		printf("[ERROR] %s. Not found version\n", __func__);
		return -1;
	}

	return 0;
}

int mozart_is_backfrom_update(void)
{
	struct nv_info info;
	int err;

	err = nvrw_get_update_info(&info);
	if (err < 0) {
		printf("[ERROR] %s. mozart Get nv info failed\n", __func__);
		return false;
	}

	/* Clear update result flag */
	if (info.update_result) {
		err = nvrw_clear_update_result();
		if (err < 0)
			printf("[ERROR] %s. mozart clear update result failed\n", __func__);
	}

	return info.update_result ? true : false;
}
