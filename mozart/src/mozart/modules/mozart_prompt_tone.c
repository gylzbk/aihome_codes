#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

#include "tips_interface.h"
#include "ini_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_prompt_tone.h"

#ifndef MOZART_RELEASE
#define MOZART_PROMPT_TONE_DEBUG
#endif

#ifdef MOZART_PROMPT_TONE_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[PROMPT_TONE] %s: "fmt, __func__, ##args)
#else  /* MOZART_PROMPT_TONE_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_PROMPT_TONE_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[PROMPT_TONE] [Error] %s: "fmt, __func__, ##args)

static int count;
static pthread_mutex_t prompt_tone_lock = PTHREAD_MUTEX_INITIALIZER;
/*******************************************************************************
 * module
 *******************************************************************************/
static void prompt_tone_set_volume(bool set)
{
#ifdef FIXED_VOLUME
	int volume = 30;
	char buf[32] = {};

	if (set) {
		volume = 50;
		mozart_volume_set(volume, TONE_VOLUME);
	} else {
		if (mozart_ini_getkey("/usr/data/system.ini", "volume", "music", buf))
			printf("failed to parse /usr/data/system.ini, set music volume to 30.\n");
		else
			volume = atoi(buf);

		mozart_volume_set(volume, MUSIC_VOLUME);
	}
#endif
}

static int prompt_tone_module_start(struct mozart_module_struct *current)
{
	return 0;
}

static int prompt_tone_module_run(struct mozart_module_struct *current)
{
	prompt_tone_set_volume(true);
	return 0;
}

static int prompt_tone_module_suspend(struct mozart_module_struct *current)
{
	prompt_tone_set_volume(false);
	return 0;
}

static int prompt_tone_module_stop(struct mozart_module_struct *current)
{
	prompt_tone_set_volume(false);
	mozart_stop_tone_sync();
	return 0;
}

#ifdef FIXED_VOLUME
static void prompt_tone_module_volume_up(struct mozart_module_struct *self)
{
}

static void prompt_tone_module_volume_down(struct mozart_module_struct *self)
{
}
#endif

static struct mozart_module_struct prompt_tone_module = {
	.name = "prompt_tone",
	.priority = 99,
	.flags = MODULE_FLAG_CHILD,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start = prompt_tone_module_start,
		.on_run = prompt_tone_module_run,
		.on_suspend = prompt_tone_module_suspend,
		.on_stop  = prompt_tone_module_stop,
	},
	.kops = {
#ifdef FIXED_VOLUME
		.volume_up = prompt_tone_module_volume_up,
		.volume_down = prompt_tone_module_volume_down,
#endif
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
static int mozart_prompt_tone_start(bool in_lock)
{
	if (prompt_tone_module.start) {
		return prompt_tone_module.start(&prompt_tone_module, module_cmd_suspend, in_lock);
	} else {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
}

static int mozart_prompt_tone_stop(bool in_lock)
{
	if (prompt_tone_module.stop) {
		return prompt_tone_module.stop(&prompt_tone_module, module_cmd_run, in_lock);
	} else {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
}

#ifdef LOCK_VOLUME
static int ref;
static pthread_mutex_t reference_lock = PTHREAD_MUTEX_INITIALIZER;

int mozart_prompt_tone_lock_volume(void)
{
	int ret = 0;
	int temp_volume = -1;
	char tone_volume[16] = {0};

	/* Get tone volume */
	if (!mozart_ini_getkey("/usr/data/system.ini", "volume", "tone", tone_volume))
		temp_volume = atoi(tone_volume);

	module_mutex_lock(&reference_lock);
	if (ref++ == 0) {
		ret = mozart_volume_lock_save(temp_volume);
		if (ret < 0)
			ref--;
	}
	pr_debug("ref = %d\n", ref);
	module_mutex_unlock(&reference_lock);

	return ret;
}

void mozart_prompt_tone_unlock_volume(int vol)
{
	module_mutex_lock(&reference_lock);
	ref--;
	pr_debug("ref = %d\n", ref);
	if (ref == 0 && vol >= 0)
		mozart_volume_unlock_restore(vol);
	module_mutex_unlock(&reference_lock);
}
#endif	/* LOCK_VOLUME */

#if 0
int mozart_prompt_tone_play(char *url)
{
}
#endif

int __mozart_prompt_tone_play_sync(char *url)
{
	int err;

	prompt_tone_set_volume(true);
	err = mozart_play_tone_sync(url);
	prompt_tone_set_volume(false);

	return err;
}

int mozart_prompt_tone_play_sync(char *url, bool in_lock)
{
	bool stop = false;
	int ret = mozart_prompt_tone_start(in_lock);

	pr_debug("url: %s\n", url);
	if (ret == 0) {
		module_mutex_lock(&prompt_tone_lock);
		count++;
		module_mutex_unlock(&prompt_tone_lock);

		mozart_play_tone_sync(url);

		module_mutex_lock(&prompt_tone_lock);
		count--;
		pr_debug("count = %d\n", count);
		if (count == 0)
			stop = true;
		module_mutex_unlock(&prompt_tone_lock);

		if (stop)
			mozart_prompt_tone_stop(in_lock);
	} else {
		pr_err("play %s fail!\n", url);
		mozart_prompt_tone_stop(in_lock);
	}

	return ret;
}

int __mozart_prompt_tone_key_sync(char *key)
{
	int err;

	prompt_tone_set_volume(true);
	err = mozart_play_key_sync(key);
	prompt_tone_set_volume(false);

	return err;
}

int mozart_prompt_tone_key_sync(char *key, bool in_lock)
{
	bool stop = false;
	int ret = mozart_prompt_tone_start(in_lock);

	pr_debug("key: %s\n", key);
	if (ret == 0) {
		module_mutex_lock(&prompt_tone_lock);
		count++;
		module_mutex_unlock(&prompt_tone_lock);

		mozart_play_key_sync(key);

		module_mutex_lock(&prompt_tone_lock);
		count--;
		pr_debug("count = %d\n", count);
		if (count == 0)
			stop = true;
		module_mutex_unlock(&prompt_tone_lock);

		if (stop)
			mozart_prompt_tone_stop(in_lock);
	} else {
		pr_err("play %s fail!\n", key);
		mozart_prompt_tone_stop(in_lock);
	}

	return ret;
}

static void *prompt_tone_key_func(void *args)
{
	pthread_detach(pthread_self());
	mozart_prompt_tone_key_sync((char *)args, false);

	return NULL;
}

int mozart_prompt_tone_key(char *key)
{
	pthread_t prompt_tone_pthread;

	if (pthread_create(&prompt_tone_pthread, NULL, prompt_tone_key_func, key) == -1) {
		pr_err("Create prompt tone key pthread failed: %s.\n", strerror(errno));
		return -1;
	}
//	pthread_detach(prompt_tone_pthread);

	return 0;
}

int mozart_prompt_tone_startup(void)
{
	int ret, try_cnt = 0;

	if (mozart_module_register(&prompt_tone_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	while (try_cnt++ < 6) {
		/* Sync check */
		ret = mozart_prompt_tone_play_sync(NULL, false);
		if (!ret)
			break;
		usleep(500 * 1000);
	}

	return 0;
}

int mozart_prompt_tone_shutdown(void)
{
	mozart_module_unregister(&prompt_tone_module);
	return 0;
}
