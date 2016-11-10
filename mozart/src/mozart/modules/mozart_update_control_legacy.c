/*
	mozart_update_control_legacy.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
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
#include "mozart_update_legacy.h"

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

static char *iniFile = "/usr/data/system.ini";

static pthread_mutex_t ini_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t global_version = 0;

#define UPDATE_CONTROL_TIME_FORMAT	"%a, %d %b %Y %T %Z"
static void update_version_flush(uint32_t ver, time_t time)
{
	char str_time[64];
	char str_ver[32];

	pthread_mutex_lock(&ini_mutex);

	strftime(str_time, 64, UPDATE_CONTROL_TIME_FORMAT, localtime(&time));
	sprintf(str_ver, "v%d", ver);

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

static int update_new_version_check(uint32_t new_ver)
{
	struct timeval tv;
	time_t recode_time;
	char recode_ver_str[32];
	uint32_t recode_ver;
	int err;

	if (!new_ver)
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

	recode_ver = atoi(recode_ver_str);

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

static void update_control_perform_fun(uint32_t version, void *priv)
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

static void update_control_execute(struct mozart_module_struct *self)
{
	int err;

	mozart_smartui_update_start();
	mozart_stop_tone_sync();

	err = mozart_update_start(
		update_control_perform_fun, NULL,
		NULL, NULL);
	if (err < 0) {
		pr_err("update run failed\n");
		goto err_update_start;
	}

	return;

err_update_start:
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
		global_version = 0;
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

static ResultCallBack_t update_control_new_update(uint32_t version, void *priv)
{
	int res;

	res = update_new_version_check(version);
	if (res < 0) {
		return UNRECOVERABLE_ERR;
	} else if (!res) {
		pr_info("Version %d abort.\n", version);
		mozart_smartui_update_update(false);

		return NEW_VERSION_ABORT;
	}

	pr_info("New version %d is available\n", version);

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

	if (global_version != 0) {
		global_version = 0;
		mozart_smartui_update_update(false);
	}

	return 0;
}
