#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "mozart_config.h"
#include "player_interface.h"
#include "volume_interface.h"
#if (SUPPORT_VR == VR_ATALK)
#include "vr-atalk_interface.h"
#endif

#if  SUPPORT_AISPEECH
#include "vr-speech_interface.h"
#include "aiengine_app.h"
#include "mozart_aispeech.h"
#endif


#include "sharememory_interface.h"

#include "mozart_module.h"
#include "mozart_player.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_bt_avk.h"
#include "mozart_atalk.h"
#include "mozart_atalk_cloudplayer_control.h"

#ifndef MOZART_RELEASE
#define MOZART_ATALK_CLOUDPLAYER_DEBUG
#endif

#ifdef MOZART_ATALK_CLOUDPLAYER_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[ATALK_CLOUDPLAYER] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_CLOUDPLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_CLOUDPLAYER_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[ATALK_CLOUDPLAYER] [Error] %s: "fmt, __func__, ##args)

static struct player_context context;
static int cloudplayer_resume_handler(void)
{
	int ret = 0;

	pr_debug("...\n");
	if (context.uuid == NULL) {
		if (atalk_cloudplayer_resume_player()) {
			pr_err("cloudplayer_resume fail\n");
			ret = -1;
		}
	} else {
		if (mozart_player_force_resume(context))
			pr_err("force_resume fail\n");

		free(context.uuid);
		free(context.url);
		context.uuid = NULL;
		context.url = NULL;
	}

	return ret;
}

static int cloudplayer_pause_handler(void)
{
	pr_debug("...\n");
	if (atalk_cloudplayer_pause_player()) {
		memset(&context, 0, sizeof(struct player_context));
		context = mozart_player_force_pause(NULL);
	}

	return 0;
}

void mozart_atalk_cloudplayer_update_context(char *uuid, char *url)
{
	if (context.uuid) {
		free(context.uuid);
		free(context.url);
	}

	context.uuid = strdup(uuid);
	context.url = strdup(url);
	context.pos = 0;
}

static int __atalk_cloudplayer_do_resume(struct mozart_module_struct *self)
{
	if (__mozart_module_is_run(self)) {
		cloudplayer_resume_handler();
		mozart_smartui_atalk_toggle(true);
		self->player_state = player_state_play;
		return 1;
	} else if (__mozart_module_is_start(self)) {
		mozart_smartui_atalk_toggle(true);
		self->player_state = player_state_play;
		return 0;
	} else {
		return -1;
	}
}

static int __atalk_cloudplayer_do_pause(struct mozart_module_struct *self)
{
	if (__mozart_module_is_run(self)) {
		cloudplayer_pause_handler();
		mozart_smartui_atalk_toggle(false);
		self->player_state = player_state_pause;
		return 1;
	} else if (__mozart_module_is_start(self)) {
		mozart_smartui_atalk_toggle(false);
		self->player_state = player_state_pause;
		return 0;
	} else {
		return -1;
	}
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int atalk_cloudplayer_module_start(struct mozart_module_struct *self)
{
	self->player_state = player_state_idle;

	return 0;
}

static int atalk_cloudplayer_module_run(struct mozart_module_struct * self)
{
	if (self->player_state == player_state_play)
		return cloudplayer_resume_handler();
	else
		return 0;
}

static int atalk_cloudplayer_module_suspend(struct mozart_module_struct * self)
{
	if (self->player_state == player_state_play)
		return cloudplayer_pause_handler();
	else
		return 0;
}

static int atalk_cloudplayer_module_stop(struct mozart_module_struct *self)
{
	atalk_cloudplayer_stop_player();
	mozart_smartui_atalk_toggle(false);
	self->player_state = player_state_idle;

	atalk_cloudplayer_monitor_module_cancel();

	return 0;
}

static void atalk_cloudplayer_module_previous_song(struct mozart_module_struct *self)
{
	atalk_cloudplayer_previous_music();
}

static void atalk_cloudplayer_module_volume_change(struct mozart_module_struct *self)
{
	int vol = mozart_volume_get();
	atalk_cloudplayer_volume_change(vol);
}

static void atalk_cloudplayer_module_resume_pause(struct mozart_module_struct *self)
{
	mozart_atalk_cloudplayer_do_resume_pause();
}

static void atalk_cloudplayer_module_asr_wakeup(struct mozart_module_struct *self)
{
	if (self->player_state != player_state_idle) {
		mozart_smartui_asr_start();
#if (SUPPORT_VR == VR_ATALK)
		mozart_atalk_asr_start();
		mozart_key_wakeup();
#elif (SUPPORT_VR == VR_SPEECH)
		mozart_speech_asr_start();
#endif
	}
}

static void atalk_cloudplayer_module_next_song(struct mozart_module_struct *self)
{
	atalk_cloudplayer_next_music();
}

static void atalk_cloudplayer_module_next_channel(struct mozart_module_struct *self)
{
	atalk_next_channel();
}

static void atalk_cloudplayer_module_favorite(struct mozart_module_struct *self)
{
	atalk_love_audio();
}

static void atalk_cloudplayer_module_next_module(struct mozart_module_struct *self)
{
	mozart_bt_avk_start(false);
}

static struct mozart_module_struct atalk_cloudplayer_module = {
	.name = "atalk_cloudplayer",
	.priority = 1,
	.attach = module_attach,
	.mops = {
		.on_start   = atalk_cloudplayer_module_start,
		.on_run     = atalk_cloudplayer_module_run,
		.on_suspend = atalk_cloudplayer_module_suspend,
		.on_stop    = atalk_cloudplayer_module_stop,
	},
	.kops = {
		.previous_song = atalk_cloudplayer_module_previous_song,
		.next_song = atalk_cloudplayer_module_next_song,
		.volume_change = atalk_cloudplayer_module_volume_change,
		.resume_pause = atalk_cloudplayer_module_resume_pause,
		.asr_wakeup = atalk_cloudplayer_module_asr_wakeup,
		.next_channel = atalk_cloudplayer_module_next_channel,
		.favorite = atalk_cloudplayer_module_favorite,
		.next_module = atalk_cloudplayer_module_next_module,
	},
};

/*******************************************************************************
 * monitor
 *******************************************************************************/
enum atalk_cloudplayer_monitor_stage {
	cloudplayer_monitor_stage_invalid = 0,
	cloudplayer_monitor_stage_wait,
	cloudplayer_monitor_stage_again,
	cloudplayer_monitor_stage_cancel,
	cloudplayer_monitor_stage_module_cancel,
};

static pthread_t atalk_cloudplayer_monitor_pthread;
static enum atalk_cloudplayer_monitor_stage cloudplayer_monitor_stage;
static pthread_mutex_t atalk_cloudplayer_monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t atalk_cloudplayer_monitor_cond = PTHREAD_COND_INITIALIZER;

static void *atalk_cloudplayer_monitor_func(void *args)
{
	int i;
	struct timeval now;
	struct timespec timeout;

	pthread_mutex_lock(&atalk_cloudplayer_monitor_mutex);
again:
	for (i = 0; i < 5; i++) {
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + 20;
		timeout.tv_nsec = now.tv_usec * 1000;

		pthread_cond_timedwait(&atalk_cloudplayer_monitor_cond,
				       &atalk_cloudplayer_monitor_mutex, &timeout);

		pr_debug("cloudplayer_monitor_stage = %d\n", cloudplayer_monitor_stage);
		if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_invalid) {
			break;
		} else if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_cancel) {
			cloudplayer_monitor_stage = cloudplayer_monitor_stage_invalid;
			break;
		} else if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_again) {
			cloudplayer_monitor_stage = cloudplayer_monitor_stage_wait;
			goto again;
		} else if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_wait) {
			pr_debug("atalk vendor restart\n");
			atalk_vendor_shutdown();
			atalk_vendor_startup();
		} else if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_module_cancel) {
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + 12;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&atalk_cloudplayer_monitor_cond,
					       &atalk_cloudplayer_monitor_mutex, &timeout);

			pr_debug("cancel: cloudplayer_monitor_stage = %d\n", cloudplayer_monitor_stage);
			if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_again) {
				cloudplayer_monitor_stage = cloudplayer_monitor_stage_wait;
				goto again;
			} else {
				cloudplayer_monitor_stage = cloudplayer_monitor_stage_invalid;
			}

			break;
		}
	}

	if (i >= 5)
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_invalid;

	pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);

	return NULL;
}

int create_atalk_cloudplayer_monitor_pthread(void)
{
	pthread_mutex_lock(&atalk_cloudplayer_monitor_mutex);

	pr_debug("cloudplayer_monitor_stage = %d\n", cloudplayer_monitor_stage);
	if (cloudplayer_monitor_stage != cloudplayer_monitor_stage_invalid) {
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_again;
		pthread_cond_signal(&atalk_cloudplayer_monitor_cond);
	} else {
		if (pthread_create(&atalk_cloudplayer_monitor_pthread, NULL,
				   atalk_cloudplayer_monitor_func, NULL) == -1) {
			printf("Create atalk cloudplayer monitor pthread failed: %s.\n", strerror(errno));
			pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);
			return -1;
		}
		pthread_detach(atalk_cloudplayer_monitor_pthread);
		pr_debug("Create atalk cloudplayer monitor pthread\n");
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_wait;
	}

	pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);

	return 0;
}

void atalk_cloudplayer_monitor_cancel(void)
{
	pthread_mutex_lock(&atalk_cloudplayer_monitor_mutex);

	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_wait ||
	    cloudplayer_monitor_stage == cloudplayer_monitor_stage_again)
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_cancel;
	pthread_cond_signal(&atalk_cloudplayer_monitor_cond);

	pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);
}

void atalk_cloudplayer_monitor_module_cancel(void)
{
	pthread_mutex_lock(&atalk_cloudplayer_monitor_mutex);

	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_wait ||
	    cloudplayer_monitor_stage == cloudplayer_monitor_stage_again)
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_module_cancel;
	pthread_cond_signal(&atalk_cloudplayer_monitor_cond);

	pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);
}

/* 切换到cloudplayer模式, 但还没收到hi_12, 说明vendor还是无效的 */
bool atalk_cloudplayer_monitor_is_valid(void)
{
	enum atalk_cloudplayer_monitor_stage stage = cloudplayer_monitor_stage;

	return (stage != cloudplayer_monitor_stage_wait) && (stage != cloudplayer_monitor_stage_again);
}

bool atalk_cloudplayer_monitor_is_module_cancel(void)
{
	bool is_module_cancel = false;

	pthread_mutex_lock(&atalk_cloudplayer_monitor_mutex);
	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_module_cancel) {
		is_module_cancel = true;
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_invalid;
	}
	pthread_mutex_unlock(&atalk_cloudplayer_monitor_mutex);

	pr_debug("is_module_cancel = %d\n", is_module_cancel);
	return is_module_cancel;
}

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_atalk_cloudplayer_is_run(void)
{
	return __mozart_module_is_run(&atalk_cloudplayer_module);
}

bool __mozart_atalk_cloudplayer_is_start(void)
{
	return __mozart_module_is_start(&atalk_cloudplayer_module);
}

int mozart_atalk_cloudplayer_start(bool in_lock)
{
	int ret = -1;

	if (atalk_cloudplayer_module.start) {
		ret = atalk_cloudplayer_module.start(&atalk_cloudplayer_module, module_cmd_stop, in_lock);
	} else {
		pr_err("atalk_cloudplayer_module isn't registered!\n");
		return -1;
	}

	if (ret == 0)
		create_atalk_cloudplayer_monitor_pthread();

#if (SUPPORT_VR == VR_SPEECH)
		ai_set_enable(true);
#endif

	return ret;
}

int mozart_atalk_cloudplayer_do_play(void)
{
	int ret;
	struct mozart_module_struct *self = &atalk_cloudplayer_module;

	mozart_module_mutex_lock();

	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_play;
		mozart_smartui_atalk_toggle(true);
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_play;
		mozart_smartui_atalk_toggle(true);
	} else {
		ret = -1;
	}

	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_cloudplayer_do_resume(void)
{
	int ret;

	mozart_module_mutex_lock();
	ret = __atalk_cloudplayer_do_resume(&atalk_cloudplayer_module);
	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_cloudplayer_do_pause(void)
{
	int ret;

	mozart_module_mutex_lock();
	ret = __atalk_cloudplayer_do_pause(&atalk_cloudplayer_module);
	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_cloudplayer_do_resume_pause(void)
{
	int ret = 0;
	struct mozart_module_struct *self = &atalk_cloudplayer_module;

	mozart_module_mutex_lock();

	if (self->player_state == player_state_play)
		ret = __atalk_cloudplayer_do_pause(self);
	else if (self->player_state == player_state_pause)
		ret = __atalk_cloudplayer_do_resume(self);

	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_cloudplayer_do_stop(void)
{
	int ret = -1;
	struct mozart_module_struct *self = &atalk_cloudplayer_module;

	mozart_module_mutex_lock();

	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_idle;
		mozart_smartui_atalk_toggle(false);
		atalk_cloudplayer_stop_player();
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_idle;
		mozart_smartui_atalk_toggle(false);
	}

	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_cloudplayer_startup(void)
{
	if (mozart_module_register(&atalk_cloudplayer_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	atalk_cloudplayer_startup();

	return 0;
}

int mozart_atalk_cloudplayer_shutdown(void)
{
	if (atalk_cloudplayer_module.stop)
		atalk_cloudplayer_module.stop(&atalk_cloudplayer_module, module_cmd_stop, false);
	mozart_module_unregister(&atalk_cloudplayer_module);
	atalk_cloudplayer_shutdown();

	return 0;
}
