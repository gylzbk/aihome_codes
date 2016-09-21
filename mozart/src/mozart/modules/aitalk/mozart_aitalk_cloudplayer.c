#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "player_interface.h"
#include "volume_interface.h"

#include "mozart_config.h"

#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"

#include "sharememory_interface.h"

#include "mozart_module.h"
#include "mozart_player.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_bt_avk.h"
#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"

#ifndef MOZART_RELEASE
#define MOZART_AITALK_CLOUDPLAYER_DEBUG
#endif

#ifdef MOZART_AITALK_CLOUDPLAYER_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[AITALK_CLOUDPLAYER] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_CLOUDPLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_CLOUDPLAYER_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[AITALK_CLOUDPLAYER] [Error] %s: "fmt, __func__, ##args)

int mozart_aitalk_cloudplayer_do_resume_pause(void);

extern bool is_aitalk_asr;
static struct player_context context;
static int cloudplayer_resume_handler(void)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	int ret = 0;
	if (context.uuid == NULL) {
		if (aitalk_cloudplayer_resume_player()) {
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
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	pr_debug("...\n");
	if (aitalk_cloudplayer_pause_player()) {
		memset(&context, 0, sizeof(struct player_context));
		context = mozart_player_force_pause(NULL);
	}

	return 0;
}

void mozart_aitalk_cloudplayer_update_context(char *uuid, char *url)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	if ((uuid == NULL)||(url == NULL)){
		return;
	}

	if (context.uuid) {
		free(context.uuid);
		context.uuid = NULL;
		free(context.url);
		context.url = NULL;
	}

	context.uuid = strdup(uuid);
	context.url = strdup(url);
	context.pos = 0;
}

static int __aitalk_cloudplayer_do_resume(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
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

static int __aitalk_cloudplayer_do_pause(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
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
static int aitalk_cloudplayer_module_start(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	self->player_state = player_state_idle;
	mozart_smartui_atalk_play("AISPEECH",NULL,NULL,NULL);
	mozart_smartui_atalk_toggle(false);
	__mozart_prompt_tone_key_sync("atalk_hi_12");
	aitalk_vendor_startup();
	return 0;
}

static int aitalk_cloudplayer_module_run(struct mozart_module_struct * self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	if (self->player_state == player_state_play)
		return cloudplayer_resume_handler();
	else
		return 0;
}

static int aitalk_cloudplayer_module_suspend(struct mozart_module_struct * self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	if (self->player_state == player_state_play)
		return cloudplayer_pause_handler();
	else
		return 0;
}

static int aitalk_cloudplayer_module_stop(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	aitalk_cloudplayer_stop_player();
	mozart_smartui_atalk_toggle(false);
	aitalk_vendor_shutdown();
	if(is_aitalk_asr){
		mozart_smartui_asr_over();
		is_aitalk_asr = false;
	}

	self->player_state = player_state_idle;

//	aitalk_cloudplayer_monitor_module_cancel();

	return 0;
}

static void aitalk_cloudplayer_module_volume_change(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	int vol = mozart_volume_get();
	aitalk_cloudplayer_volume_change(vol);
}

static void aitalk_cloudplayer_module_resume_pause(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	mozart_aitalk_cloudplayer_do_resume_pause();
}

static void aitalk_cloudplayer_module_asr_wakeup(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);

	//if (self->player_state != player_state_idle) {
	//	mozart_aitalk_cloudplayer_do_pause();
	//}
	//mozart_smartui_asr_start();
	//mozart_aitalk_asr_start();
	//ai_sem_start();
	//mozart_prompt_tone_key_sync("welcome", false);
}

static void aitalk_cloudplayer_module_previous_song(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	ai_play_music_order(-1);
	usleep(500000);
}

static void aitalk_cloudplayer_module_next_song(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	ai_play_music_order(1);
	usleep(500000);
}

static void aitalk_cloudplayer_module_next_channel(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	aitalk_next_channel();
}

static void aitalk_cloudplayer_module_favorite(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	aitalk_love_audio();
}

static void aitalk_cloudplayer_module_next_module(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	mozart_bt_avk_start(false);
}


extern bool aitalk_is_playing;
static bool aitalk_cloudplayer_module_is_playing(struct mozart_module_struct *self)
{
	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	return aitalk_is_playing;
}

static struct mozart_module_struct aitalk_cloudplayer_module = {
	.name = "aitalk_cloudplayer",
	.priority = 1,
	.attach = module_attach,
	.mops = {
		.on_start   = aitalk_cloudplayer_module_start,
		.on_run     = aitalk_cloudplayer_module_run,
		.on_suspend = aitalk_cloudplayer_module_suspend,
		.on_stop    = aitalk_cloudplayer_module_stop,
	},
	.kops = {
		.previous_song = aitalk_cloudplayer_module_previous_song,
		.next_song = aitalk_cloudplayer_module_next_song,
		.volume_change = aitalk_cloudplayer_module_volume_change,
		.resume_pause = aitalk_cloudplayer_module_resume_pause,
		.asr_wakeup = aitalk_cloudplayer_module_asr_wakeup,
		.next_channel = aitalk_cloudplayer_module_next_channel,
		.favorite = aitalk_cloudplayer_module_favorite,
		.next_module = aitalk_cloudplayer_module_next_module,
		.is_playing = aitalk_cloudplayer_module_is_playing,
	},
};
#if 0
/*******************************************************************************
 * monitor
 *******************************************************************************/
enum aitalk_cloudplayer_monitor_stage {
	cloudplayer_monitor_stage_invalid = 0,
	cloudplayer_monitor_stage_wait,
	cloudplayer_monitor_stage_again,
	cloudplayer_monitor_stage_cancel,
	cloudplayer_monitor_stage_module_cancel,
};

static pthread_t aitalk_cloudplayer_monitor_pthread;
static enum aitalk_cloudplayer_monitor_stage cloudplayer_monitor_stage;
static pthread_mutex_t aitalk_cloudplayer_monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t aitalk_cloudplayer_monitor_cond = PTHREAD_COND_INITIALIZER;

static void *aitalk_cloudplayer_monitor_func(void *args)
{
	pthread_detach(pthread_self());
	int i;
	struct timeval now;
	struct timespec timeout;

	pthread_mutex_lock(&aitalk_cloudplayer_monitor_mutex);
again:
	for (i = 0; i < 5; i++) {
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + 20;
		timeout.tv_nsec = now.tv_usec * 1000;

		pthread_cond_timedwait(&aitalk_cloudplayer_monitor_cond,
				       &aitalk_cloudplayer_monitor_mutex, &timeout);

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
			pr_debug("aitalk vendor restart\n");
			aitalk_vendor_shutdown();
			aitalk_vendor_startup();
		} else if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_module_cancel) {
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + 12;
			timeout.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&aitalk_cloudplayer_monitor_cond,
					       &aitalk_cloudplayer_monitor_mutex, &timeout);

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

	pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);

	return NULL;
}

int create_aitalk_cloudplayer_monitor_pthread(void)
{
	pthread_mutex_lock(&aitalk_cloudplayer_monitor_mutex);

	pr_debug("cloudplayer_monitor_stage = %d\n", cloudplayer_monitor_stage);
	if (cloudplayer_monitor_stage != cloudplayer_monitor_stage_invalid) {
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_again;
		pthread_cond_signal(&aitalk_cloudplayer_monitor_cond);
	} else {
		if (pthread_create(&aitalk_cloudplayer_monitor_pthread, NULL,
				   aitalk_cloudplayer_monitor_func, NULL) == -1) {
			printf("Create atalk cloudplayer monitor pthread failed: %s.\n", strerror(errno));
			pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);
			return -1;
		}
//		pthread_detach(aitalk_cloudplayer_monitor_pthread);
		pr_debug("Create atalk cloudplayer monitor pthread\n");
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_wait;
	}

	pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);

	return 0;
}

void aitalk_cloudplayer_monitor_cancel(void)
{
	pthread_mutex_lock(&aitalk_cloudplayer_monitor_mutex);

	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_wait ||
	    cloudplayer_monitor_stage == cloudplayer_monitor_stage_again)
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_cancel;
	pthread_cond_signal(&aitalk_cloudplayer_monitor_cond);

	pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);
}

void aitalk_cloudplayer_monitor_module_cancel(void)
{
	pthread_mutex_lock(&aitalk_cloudplayer_monitor_mutex);

	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_wait ||
	    cloudplayer_monitor_stage == cloudplayer_monitor_stage_again)
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_module_cancel;
	pthread_cond_signal(&aitalk_cloudplayer_monitor_cond);

	pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);
}

/* 切换到cloudplayer模式, 但还没收到hi_12, 说明vendor还是无效的 */
bool aitalk_cloudplayer_monitor_is_valid(void)
{
	enum aitalk_cloudplayer_monitor_stage stage = cloudplayer_monitor_stage;

	return (stage != cloudplayer_monitor_stage_wait) && (stage != cloudplayer_monitor_stage_again);
}

bool aitalk_cloudplayer_monitor_is_module_cancel(void)
{
	bool is_module_cancel = false;

	pthread_mutex_lock(&aitalk_cloudplayer_monitor_mutex);
	if (cloudplayer_monitor_stage == cloudplayer_monitor_stage_module_cancel) {
		is_module_cancel = true;
		cloudplayer_monitor_stage = cloudplayer_monitor_stage_invalid;
	}
	pthread_mutex_unlock(&aitalk_cloudplayer_monitor_mutex);

	pr_debug("is_module_cancel = %d\n", is_module_cancel);
	return is_module_cancel;
}
#endif
/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_aitalk_cloudplayer_is_run(void)
{
	return __mozart_module_is_run(&aitalk_cloudplayer_module);
}

bool __mozart_aitalk_cloudplayer_is_start(void)
{
	return __mozart_module_is_start(&aitalk_cloudplayer_module);
}


bool __mozart_aitalk_cloudplayer_is_asr(void){
	if (__mozart_aitalk_cloudplayer_is_run())
		return is_aitalk_asr;
	else
		return false;
}

int mozart_aitalk_cloudplayer_start(bool in_lock)
{
	int ret = -1;

	pr_debug("\n====================== %d %s \n\n",__LINE__,__func__);
	if (aitalk_cloudplayer_module.start) {
		ret = aitalk_cloudplayer_module.start(&aitalk_cloudplayer_module, module_cmd_stop, in_lock);
	} else {
		pr_err("aitalk_cloudplayer_module isn't registered!\n");
		return -1;
	}
//	aitalk_vendor_startup();
//	if (ret == 0)
//		create_aitalk_cloudplayer_monitor_pthread();

	return ret;
}

int mozart_aitalk_cloudplayer_do_play(void)
{
	int ret;
	struct mozart_module_struct *self = &aitalk_cloudplayer_module;

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

int mozart_aitalk_cloudplayer_do_resume(void)
{
	int ret;

	mozart_module_mutex_lock();
	ret = __aitalk_cloudplayer_do_resume(&aitalk_cloudplayer_module);
	mozart_module_mutex_unlock();

	return ret;
}

int mozart_aitalk_cloudplayer_do_pause(void)
{
	int ret;

	mozart_module_mutex_lock();
	ret = __aitalk_cloudplayer_do_pause(&aitalk_cloudplayer_module);
	mozart_module_mutex_unlock();

	return ret;
}

int mozart_aitalk_cloudplayer_do_resume_pause(void)
{
	int ret = 0;
	struct mozart_module_struct *self = &aitalk_cloudplayer_module;

	mozart_module_mutex_lock();

	if (self->player_state == player_state_play)
		ret = __aitalk_cloudplayer_do_pause(self);
	else if (self->player_state == player_state_pause)
		ret = __aitalk_cloudplayer_do_resume(self);

	mozart_module_mutex_unlock();

	return ret;
}

int mozart_aitalk_cloudplayer_do_stop(void)
{
	int ret = -1;
	struct mozart_module_struct *self = &aitalk_cloudplayer_module;

	mozart_module_mutex_lock();

	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_idle;
		mozart_smartui_atalk_toggle(false);
		aitalk_cloudplayer_stop_player();
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_idle;
		mozart_smartui_atalk_toggle(false);
	}

	mozart_module_mutex_unlock();

	return ret;
}

int mozart_aitalk_cloudplayer_startup(void)
{

	pr_debug("=======================> mozart_aitalk_cloudplayer_startup...!\n");
	if (mozart_module_register(&aitalk_cloudplayer_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
	aitalk_cloudplayer_startup();
	return 0;
}

int mozart_aitalk_cloudplayer_shutdown(void)
{
	pr_debug("=======================> mozart_aitalk_cloudplayer_shutdown...!\n");
	if (aitalk_cloudplayer_module.stop)
		aitalk_cloudplayer_module.stop(&aitalk_cloudplayer_module, module_cmd_stop, false);
	mozart_module_unregister(&aitalk_cloudplayer_module);
	aitalk_cloudplayer_shutdown();
	return 0;
}


