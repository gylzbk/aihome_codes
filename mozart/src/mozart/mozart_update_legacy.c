/*
	mozart_update_legacy.c
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/stat.h>

#include "utils_interface.h"
#include "ini_interface.h"
#include "ota_interface.h"

#include "mozart_update_legacy.h"

struct update_t {
	pthread_t	pthread;

	int		msg_pipe[2];
	long		interval_time; /* Unit: second */

	int		update_ver;

	/* Newer version found callbak */
	ResultCallBack_t (*update_new_callback)(uint32_t version, void *priv);
	void *priv_data;
};

static struct update_t mozart_update = {
	.msg_pipe		= {-1, -1},
	.update_ver		= 0,
	.update_new_callback	= NULL,
};


static char infoFile[] = "/tmp/update-info";
static char iniFile[] = "/usr/data/system.ini";
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
	return update_remote_get("info", target);
}

static int update_info_parse_version(char *file, uint32_t *version)
{
	char *line = NULL;
	size_t len = 0;;
	ssize_t rSize;
	char *value;
	FILE *fp;

	fp = fopen(file, "rb");
	if (!fp) {
		printf("[WARNING] %s. open %s: %s\n", __func__, file, strerror(errno));
		return -1;
	}

	while ((rSize = getline(&line, &len, fp)) != -1) {
		if (rSize) {
			value = strchr(line, '=');
			if (value) {
				if (strstr(line, "version")) {
					value += 1;
					*version = (uint32_t)atoi(value);
					break;
				}
			}
		}
	}

	if (line)
		free(line);
	fclose(fp);

	return rSize != -1 ? 0 : -1;
}

static int update_check_version(int remote, int current)
{
	return remote > current;
}

static void *update_check_func(void *data)
{
	pthread_detach(pthread_self());
	uint32_t current_ver, new_ver;
	int fd;
	int retval;
	int err;

	fd_set fds;
	struct timeval tv = {
		.tv_sec		= 0,
		.tv_usec	= 0,
	};

	/* Get local version */
	fd = mozart_ota_init();
	if (fd < 0) {
		printf("[ERROR] %s. mozart OTA init: %s\n", __func__, strerror(errno));
		return NULL;
	}
	current_ver = mozart_ota_get_version(fd);
	mozart_ota_close(fd);

	printf("[Info] %s, Mozart version: %d\n", __func__, current_ver);

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

			/* parse version */
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
			printf("<+> update check turn back\n");
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
	ResultCallBack_t (*newer_callback)(uint32_t, void *),
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
	if (err < 0) {
		printf("[ERROR] %s. Create update check thread: %s\n", __func__, strerror(errno));
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

static int update_prepear(
	void (*tsize_callback)(char *, int, void *),
	void *tsize_priv)
{
	uint32_t ver;
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

	mozart_update.update_ver = ver;

	return 0;
}

static int update_active(void)
{
	int fd;
	int err;

	err = mozart_ini_getkey(iniFile, "Update", "remote-url", otaUrl);
	if (err) {
		printf("[ERROR] %s. Not get remote-url\n", __func__);
		return -1;
	}

	fd = mozart_ota_init();
	if (fd < 0) {
		printf("[ERROR] %s. mozart OTA init: %s\n", __func__, strerror(errno));
		return -1;
	}

	mozart_ota_seturl(otaUrl, strlen(otaUrl), fd);
	mozart_ota_start_update();
	printf("Mozart update OTA standby ...\n");

	mozart_ota_close(fd);

	/* Reboot system */
	mozart_system("reboot -d 4");

	return 0;
}

int mozart_update_start(
	void (*perform_callback)(uint32_t, void *),
	void *per_priv,
	void (*tsize_callback)(char *, int, void *),
	void *tsize_priv)
{
	if (update_prepear(tsize_callback, tsize_priv) < 0)
		return -1;

	if (perform_callback)
		perform_callback(mozart_update.update_ver, per_priv);

	return update_active();
}

int mozart_update_cancel(void)
{
	return 0;
}

int mozart_is_backfrom_update(void)
{
	return 0;
}
