/*
	mozart_update_control.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "ini_interface.h"
#include "tips_interface.h"

#include "mozart_module.h"
#include "mozart_misc.h"
#include "mozart_prompt_tone.h"
#include "mozart_smartui.h"
#include "mozart_update.h"

#include "mozart_update_control.h"

#ifndef MOZART_RELEASE
#define MOZART_UPDATE_CONTROL_DEBUG
#endif

#ifdef MOZART_UPDATE_CONTROL_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[UPDATE_CONTROL] %s: "fmt, __func__, ##args)
#else  /* MOZART_UPDATE_CONTROL_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_UPDATE_CONTROL_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[UPDATE_CONTROL] [Error] %s: "fmt, __func__, ##args)

#define pr_info(fmt, args...)			\
	printf("[UPDATE_CONTROL] [Info] %s: "fmt, __func__, ##args)

typedef struct {
	pthread_t	pthread;
	int		msg_pipe[2];

	char		*dl_file;
	int		total_size;
	int		percent_old;
} updateProgress_t;

static char *iniFile = "/usr/data/system.ini";

static pthread_mutex_t ini_mutex = PTHREAD_MUTEX_INITIALIZER;
static float global_version = 0.0;

#define UPDATE_CONTROL_TIME_FORMAT	"%a, %d %b %Y %T %Z"
static void update_version_flush(float ver, time_t time)
{
	char str_time[64];
	char str_ver[32];

	pthread_mutex_lock(&ini_mutex);

	strftime(str_time, 64, UPDATE_CONTROL_TIME_FORMAT, localtime(&time));
	sprintf(str_ver, "v%.5f", ver);

	mozart_ini_setkey(iniFile, "NewUpdate", "version", str_ver);
	mozart_ini_setkey(iniFile, "NewUpdate", "date", str_time);

	pthread_mutex_unlock(&ini_mutex);
}

static int update_get_recode_time(time_t *rec_time)
{
	struct tm tm;
	char str_time[64];

	pthread_mutex_lock(&ini_mutex);

	if (mozart_ini_getkey(iniFile, "NewUpdate", "date", str_time))
		return -1;

	strptime(str_time, UPDATE_CONTROL_TIME_FORMAT, &tm);
	*rec_time = mktime(&tm);

	pthread_mutex_unlock(&ini_mutex);

	return 0;
}
#undef UPDATE_CONTROL_TIME_FORMAT

#define UPDATE_CONTROL_REFTIME		"1 Jan 2000 00:00:00"
static inline int systime_is_valid(time_t time)
{
	struct tm tm;
	strptime(UPDATE_CONTROL_REFTIME, "%d %b %Y %T", &tm);
	return time >= mktime(&tm) ? true : false;
}
#undef UPDATE_CONTROL_REFTIME

static int update_new_version_check(float new_ver)
{
	struct timeval tv;
	time_t recode_time;
	char recode_ver_str[32];
	float recode_ver;
	int err;

	if (new_ver == 0.0)
		return 0;

	err = gettimeofday(&tv, NULL);
	if (err < 0) {
		pr_err("gettimeofday: %s\n", strerror(errno));
		return -1;
	}

	if (!systime_is_valid(tv.tv_sec)) {
		pr_err("Systime not valid\n");
		return 1;
	}

	if (mozart_ini_getkey(iniFile, "NewUpdate", "version", recode_ver_str)) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	}

	recode_ver = strtof(recode_ver_str + 1, NULL);

	if (new_ver > recode_ver) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	} else if (new_ver < recode_ver) {
		pr_err("Recode version is upper than new version\n");
		update_version_flush(new_ver, tv.tv_sec);
		return -1;
	}

	if (update_get_recode_time(&recode_time)) {
		update_version_flush(new_ver, tv.tv_sec);
		return 1;
	}

	return 1;
}

static void *update_control_play_prompt_func(void *data)
{
	mozart_prompt_tone_key_sync("atalk_update_new_31", false);
	return NULL;
}

static int update_control_prompt(void)
{
	pthread_t prompt_thread;
	int err;

	err = pthread_create(&prompt_thread, NULL, update_control_play_prompt_func, NULL);
	if (err < 0) {
		pr_err("Create pthread: %s\n", strerror(errno));
		return -1;
	}
	pthread_detach(prompt_thread);

	return 0;
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int update_control_module_start(struct mozart_module_struct *current)
{
	mozart_smartui_update();
	return update_control_prompt();
}

static int update_control_module_run(struct mozart_module_struct *current)
{
	return 0;
}

static int update_control_module_suspend(struct mozart_module_struct *current)
{
	return 0;
}

static int update_control_module_stop(struct mozart_module_struct *current)
{
	return 0;
}

static void update_control_module_wifi_config(struct mozart_module_struct *self)
{
	/* Don't support */
}

static void update_control_perform_fun(float version, void *priv)
{
	struct timeval tv;
	int err;

	/* Flush update info */
	err = gettimeofday(&tv, NULL);
	if (err < 0) {
		pr_err("gettimeofday: %s\n", strerror(errno));
		return;
	}

	if (!systime_is_valid(tv.tv_sec)) {
		pr_err("Systime not valid\n");
		return;
	}

	update_version_flush(version, tv.tv_sec);
}

static void *update_control_progress_func(void *data)
{
	updateProgress_t *progress = (updateProgress_t *)data;
	struct stat st;
	int percent;
	int retval;
	int err;

	fd_set fds;
	struct timeval tv = {
		.tv_sec		= 2,
		.tv_usec	= 0,
	};

	for (;;) {
		/* Clear select fds */
		FD_ZERO(&fds);
		/* Set pipe read end */
		FD_SET(progress->msg_pipe[0], &fds);

		tv.tv_sec = 2;
		retval = select(progress->msg_pipe[0] + 1, &fds, NULL, NULL, &tv);
		if (retval < 0) {
			printf("[ERROR] %s. select: %s\n", __func__, strerror(errno));
			goto err_select;
		}

		if (!retval) {
			err = stat(progress->dl_file, &st);
			if (err < 0) {
				if (errno == ENOENT) {
					pr_debug("%s has not created\n", progress->dl_file);
					continue;
				} else {
					pr_err("Get %s state: %s\n", progress->dl_file, strerror(errno));
					goto err_dl_stat;
				}
			}

			percent = ((st.st_size + 1023) / 1024) * 100 / progress->total_size;
			if (percent > 95)
				percent = 100;

			if (percent != progress->percent_old) {
				/* pr_debug("update percent: %d\n", percent); */

				/* smart ui progress display*/
				mozart_smartui_update_progress(percent);
			}

			progress->percent_old = percent;
			if (percent == 100)
				break;
		} else {
			if (FD_ISSET(progress->msg_pipe[0], &fds)) {
				char msg = 0;
				read(progress->msg_pipe[0], &msg, sizeof(msg));
				/* Quit update progress */
				break;
			}
		}
	}

err_dl_stat:
err_select:
	return NULL;
}

static void update_control_tsize_func(char *dl_file, int total_size, void *priv)
{
	updateProgress_t *progress = (updateProgress_t *)priv;
	int err;

	if (!dl_file) {
		pr_err("dl_file is NULL\n");
		return;
	}

	if (!total_size) {
		pr_err("Download update file total size is 0\n");
		return;
	}

	progress->dl_file = strdup(dl_file);
	if (!progress->dl_file) {
		pr_err("Strdup file: %s\n", strerror(errno));
		return;
	}

	progress->total_size	= total_size; /* Unit: Kbytes */
	progress->percent_old	= -1;

	err = pipe(progress->msg_pipe);
	if (err < 0) {
		pr_err("Get msg pipe: %s\n", strerror(errno));
		goto err_pipe;
	}

	err = pthread_create(&progress->pthread, NULL, update_control_progress_func, progress);
	if (err) {
		pr_err("Create progress pthread: %s\n", strerror(errno));
		goto err_thread_create;
	}

	return;

err_thread_create:
	close(progress->msg_pipe[0]);
	progress->msg_pipe[0] = -1;
	close(progress->msg_pipe[1]);
	progress->msg_pipe[1] = -1;

err_pipe:
	free(progress->dl_file);
	progress->dl_file = NULL;
}

static void update_control_progress_stop(updateProgress_t *progress)
{
	char msg = 'Q';

	if (progress->msg_pipe[0] < 0 ||
	    progress->msg_pipe[1] < 0)
		return;

	/* Send poll quit message to mdns pthread */
	write(progress->msg_pipe[1], &msg, sizeof(msg));
	/* Wait pthread stoped */
	pthread_join(progress->pthread, NULL);

	close(progress->msg_pipe[0]);
	close(progress->msg_pipe[1]);

	free(progress->dl_file);
}

static void update_control_execute(struct mozart_module_struct *self)
{
	updateProgress_t *progress;
	int err;

	mozart_smartui_update_start();
	mozart_stop_tone_sync();

	if (!mozart_ext_storage_enough_free("/mnt/sdcard", 128 * 1024 * 1024)) {
		pr_info("Has not enough space, clear atalk-favorite\n");
		mozart_ext_storage_file_clear("/mnt/sdcard/atalk-favorite");
	}

	progress = malloc(sizeof(updateProgress_t));
	if (!progress) {
		pr_err("Alloc progress struct: %s\n", strerror(errno));
		goto err_progress_alloc;
	}

	err = mozart_update_start(
		update_control_perform_fun, NULL,
		update_control_tsize_func, progress);
	update_control_progress_stop(progress);
	if (err < 0) {
		pr_err("update run failed\n");
		goto err_update_start;
	}

	free(progress);

	return;

err_update_start:
	free(progress);

err_progress_alloc:
	mozart_prompt_tone_key_sync("atalk_update_fail_33", false);
	mozart_smartui_update_cancel();
	self->stop(self, module_cmd_run, false);
}

static void update_control_cancel(struct mozart_module_struct *self)
{
	mozart_smartui_update_cancel();
	mozart_stop_tone_sync();
	self->stop(self, module_cmd_run, false);
}

static struct mozart_module_struct update_control_module = {
	.name = "update-control",
	.priority = 10,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start  = update_control_module_start,
		.on_run     = update_control_module_run,
		.on_suspend = update_control_module_suspend,
		.on_stop    = update_control_module_stop,
	},
	.kops = {
		.wifi_config = update_control_module_wifi_config,
		.next_song = update_control_execute,
		.favorite = update_control_cancel,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
int mozart_update_control_try_start(bool in_lock)
{
	int res;

	res = update_new_version_check(global_version);
	if (res < 0) {
		return -1;
	} else if (!res) {
		global_version = 0.0;
		mozart_smartui_update_update(false);
		pr_debug("New version Not found.\n");

		return -1;
	}

	if (!update_control_module.start) {
		pr_err("%s is not registered\n", update_control_module.name);
		return -1;
	}

	return update_control_module.start(&update_control_module, module_cmd_suspend, in_lock);
}

void mozart_update_control_backfrom_update(void)
{
	if (mozart_is_backfrom_update()) {
		/* Handle back from update */
		mozart_prompt_tone_key_sync("atalk_update_ok_32", false);
	}
}

static ResultCallBack_t update_control_new_update(float version, void *priv)
{
	int res;

	res = update_new_version_check(version);
	if (res < 0) {
		return UNRECOVERABLE_ERR;
	} else if (!res) {
		pr_info("Version %.5f abort.\n", version);
		mozart_smartui_update_update(false);

		return NEW_VERSION_ABORT;
	}

	pr_info("New version %.5f is available\n", version);

	global_version = version;
	mozart_smartui_update_update(true);

	return NEW_VERSION_OK;
}

int mozart_update_control_startup(void)
{
	if (mozart_module_register(&update_control_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	/* Interval time 30 minutes */
	return mozart_update_check_start(update_control_new_update, NULL, 30 * 60);
}

int mozart_update_control_shutdown(void)
{
	if (!mozart_module_unregister(&update_control_module))
		mozart_update_check_stop();

	if (global_version != 0.0) {
		global_version = 0.0;
		mozart_smartui_update_update(false);
	}

	return 0;
}
