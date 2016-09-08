/*
	mozart_update.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "utils_interface.h"
#include "ini_interface.h"
#include "ota_interface.h"
#include "mozart_update.h"

struct update_t {
	pthread_t	pthread;

	int		msg_pipe[2];
	long		interval_time; /* Unit: second */

	void (*newer_update_callback)(uint32_t version, void *prevate);
	void *priv_data;
};

static struct update_t mozart_update = {
	.msg_pipe	= {-1, -1},
};


static char otaUrl[] = "http://192.168.1.200/ota/download-atalk";
static char loadFile[] = "/tmp/update-info";
static char iniFile[] = "/usr/data/system.ini";

#define UPDATE_TIME_FORMAT	"%a, %d %b %Y %T %Z"
static void update_version_flush(uint32_t ver, time_t time)
{
	char str_time[64];
	char str_ver[11];

	strftime(str_time, 64, UPDATE_TIME_FORMAT, localtime(&time));
	snprintf(str_ver, 11, "%d", ver);

	mozart_ini_setkey(iniFile, "NewUpdate", "version", str_ver);
	mozart_ini_setkey(iniFile, "NewUpdate", "date", str_time);
}

static int update_get_recode_time(time_t *rec_time)
{
	struct tm tm;
	char str_time[64];

	if (mozart_ini_getkey(iniFile, "NewUpdate", "date", str_time))
		return -1;

	strptime(str_time, UPDATE_TIME_FORMAT, &tm);
	*rec_time = mktime(&tm);

	return 0;
}
#undef UPDATE_TIME_FORMAT

static int update_info_get(char *target)
{
	char *exep;
	int err;

	exep = malloc(strlen(otaUrl) + sizeof("/info") + 128);
	if (!exep) {
		printf("[ERROR] %s. Alloc exep: %s\n", __func__, strerror(errno));
		return -1;
	}

	unlink(target);

	/* Download update version info file */
	sprintf(exep, "wget -O %s -T %d '%s/info'", target, 5, otaUrl);
	err = mozart_system(exep);
	free(exep);

	if (err < 0) {
		printf("[ERROR] %s. Run wget failed\n", __func__);
		return -1;
	}

	return WEXITSTATUS(err);
}

static int update_info_parse_version(char *file, uint32_t *version)
{
	char *line = NULL;;
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

static int update_new_version_check(uint32_t new_ver)
{
	struct timeval tv;
	time_t recode_time;
	char recode_ver[10];
	int err;

	err = gettimeofday(&tv, NULL);
	if (err < 0) {
		printf("[ERROR] %s. gettimeofday: %s\n", __func__, strerror(errno));
		return -1;
	}

	if (mozart_ini_getkey(iniFile, "NewUpdate", "version", recode_ver)) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	}

	if (new_ver > atoi(recode_ver)) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	}

	if (update_get_recode_time(&recode_time)) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	}

	return tv.tv_sec < (recode_time + 3600 * 24 * 3) ? 1 : 0;
}

static void *update_check_func(void *data)
{
	uint32_t local_ver, new_ver;
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
	local_ver = mozart_ota_get_version(fd);
	mozart_ota_close(fd);

	printf("[Info] %s, Mozart version: %d\n", __func__, local_ver);

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
			err = update_info_get(loadFile);
			if (err < 0) {
				goto err_get_info;
			} else if (err) {
				printf("[WARNING] %s. Get info: %s\n", __func__, strerror(err));
				goto abort_this_time;
			}

			/* parse version */
			if (update_info_parse_version(loadFile, &new_ver)) {
				printf("[WARNING] %s. No version info found\n", __func__);
				/* No version info found */
				goto abort_this_time;
			}

			if (new_ver > local_ver) {
				err = update_new_version_check(new_ver);
				if (err < 0) {
					/* Unrecoverable error */
					goto err_new_version_check;
				} else if (err) {
					printf("[Attention] %s. Newer version found.\n", __func__);
					if (mozart_update.newer_update_callback)
						mozart_update.newer_update_callback(new_ver, mozart_update.priv_data);
					break;
				}
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

err_new_version_check:
err_get_info:
err_select:

	return NULL;
}

int mozart_update_check_start(
	void (*newer_callback)(uint32_t, void *),
	void *priv,
	long interval_time)
{
	int err;

	if (mozart_update.msg_pipe[0] >= 0 ||
	    mozart_update.msg_pipe[1] >= 0) {
		printf("[ERROR] %s. update check thread has running\n", __func__);
		return -1;
	}

	mozart_update.newer_update_callback	= newer_callback;
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
	/* Wait pthread stoped */
	pthread_join(mozart_update.pthread, NULL);

	/* Close pipe */
	close(mozart_update.msg_pipe[0]);
	mozart_update.msg_pipe[0] = -1;
	close(mozart_update.msg_pipe[1]);
	mozart_update.msg_pipe[1] = -1;

	return 0;
}

int mozart_update_active(void)
{
	int fd;

	fd = mozart_ota_init();
	if (fd < 0) {
		printf("%s. mozart OTA init: %s\n", __func__, strerror(errno));
		return -1;
	}

	mozart_ota_seturl(otaUrl, strlen(otaUrl), fd);
	printf("OTA standby ...\n");
	mozart_ota_start_update();

	mozart_ota_close(fd);

	return 0;
}

int mozart_update_cancel(void)
{
	return 0;
}
