#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "vr-atalk_interface.h"
#include "ini_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"

#ifndef MOZART_RELEASE
#define MOZART_ATALK_ASR_DEBUG
#endif

#ifdef MOZART_ATALK_ASR_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[ATALK_ASR] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_ASR_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_ASR_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[ATALK_ASR] [Error] %s: "fmt, __func__, ##args)

/*******************************************************************************
 * module
 *******************************************************************************/
static void atalk_asr_set_volume(bool set)
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

static int atalk_asr_module_start(struct mozart_module_struct *current)
{
	return 0;
}

static int atalk_asr_module_run(struct mozart_module_struct *current)
{
	atalk_asr_set_volume(true);
	return 0;
}

static int atalk_asr_module_suspend(struct mozart_module_struct *current)
{
	atalk_asr_set_volume(false);
	return 0;
}

static int atalk_asr_module_stop(struct mozart_module_struct *current)
{
	atalk_asr_set_volume(false);
	return 0;
}

#ifdef FIXED_VOLUME
static void atalk_asr_module_volume_up(struct mozart_module_struct *self)
{
}

static void atalk_asr_module_volume_down(struct mozart_module_struct *self)
{
}
#endif

static void atalk_asr_module_wifi_config(struct mozart_module_struct *current)
{
	/* Don't support */
}

static void atalk_asr_module_asr_cancel(struct mozart_module_struct *current)
{
	mozart_vr_atalk_key_cancel();
}

static struct mozart_module_struct atalk_asr_module = {
	.name = "atalk_asr",
	.priority = 9,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = atalk_asr_module_start,
		.on_run     = atalk_asr_module_run,
		.on_suspend = atalk_asr_module_suspend,
		.on_stop    = atalk_asr_module_stop,
	},
	.kops = {
#ifdef FIXED_VOLUME
		.volume_up = atalk_asr_module_volume_up,
		.volume_down = atalk_asr_module_volume_down,
#endif
		.wifi_config = atalk_asr_module_wifi_config,
		.asr_cancel = atalk_asr_module_asr_cancel,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
int mozart_atalk_asr_start(void)
{
	if (atalk_asr_module.start) {
		return atalk_asr_module.start(&atalk_asr_module, module_cmd_suspend, false);
	} else {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
}

static int mozart_vr_atalk_interface_callback(void *arg)
{
	enum asr_state_type asr_state = (enum asr_state_type)arg;

	if (asr_state == ASR_STATE_RECOG)
		;
	else if (asr_state == ASR_STATE_IDLE)
		;
	else
		printf("%s: asr_state = %d?\n", __func__, asr_state);

	return 0;
}

static void *stop_func(void *args)
{
	pthread_detach(pthread_self());
	usleep(500 * 1000);
	mozart_smartui_asr_over();
	if (atalk_asr_module.stop)
		atalk_asr_module.stop(&atalk_asr_module, module_cmd_run, false);
	else
		pr_err("mozart_module_register fail\n");

	return NULL;
}

int mozart_atalk_asr_over(void)
{
	pthread_t stop_pthread;

	if (pthread_create(&stop_pthread, NULL, stop_func, NULL) == -1) {
		pr_err("create display delay fail\n");
		return -1;
	}
	//pthread_detach(stop_pthread);

	return 0;
}

int mozart_atalk_asr_startup(void)
{
	if (mozart_module_register(&atalk_asr_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	mozart_vr_atalk_startup(KEY_LONGPRESS_WAKEUP, mozart_vr_atalk_interface_callback);

	return 0;
}

int mozart_atalk_asr_shutdown(void)
{
	mozart_module_unregister(&atalk_asr_module);
	mozart_vr_atalk_shutdown();

	return 0;
}
