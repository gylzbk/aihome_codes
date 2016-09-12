#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "player_interface.h"
#include "tips_interface.h"
#include "localplayer_interface.h"
#include "sharememory_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_player.h"
#include "mozart_bt_avk.h"
#include "mozart_net.h"

#ifndef MOZART_RELEASE
#define MOZART_ATALK_LOCALPLAYER_DEBUG
#endif

#ifdef MOZART_ATALK_LOCALPLAYER_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[ATALK_LOCALPLAYER] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_LOCALPLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_LOCALPLAYER_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[ATALK_LOCALPLAYER] [Error] %s: "fmt, __func__, ##args)

static bool localplayer_starting;
static struct player_context context;

static void *localplayer_start_play(void *args)
{

	pthread_detach(pthread_self());
	mozart_localplayer_start_playback();
	localplayer_starting = false;

	return NULL;
}

static void create_localplayer_start_play(void)
{
	int err;
	pthread_t localplayer_start_play_thread;

	if (localplayer_starting)
		return ;
	else
		localplayer_starting = true;

	/* prevent deadlock */
	err = pthread_create(&localplayer_start_play_thread, NULL, localplayer_start_play, NULL);
	if (err < 0) {
		pr_err("Create pthread: %s\n", strerror(errno));
		return ;
	}
//	pthread_detach(localplayer_start_play_thread);

	return ;
}

static int localplayer_resume_handler(void)
{
	int ret = 0;
	player_handler_t *handler = NULL;
	player_status_t status = PLAYER_UNKNOWN;

	pr_debug("...\n");
	if (context.uuid == NULL) {
		handler = mozart_player_handler_get("player_saver", NULL, NULL);
		if (!handler) {
			pr_err("player handler get\n");
			return -1;
		}

		status = mozart_player_getstatus(handler);
		if (status == PLAYER_PAUSED) {
			if (mozart_localplayer_play_pause()) {
				pr_err("localplayer_resume_pause fail\n");
				ret = -1;
			}
		} else {
			pr_debug("player is %d\n", status);
		}

		mozart_player_handler_put(handler);
	} else {
		if (mozart_player_force_resume(context))
			pr_debug("force_resume fail\n");

		free(context.uuid);
		free(context.url);
		context.uuid = NULL;
		context.url = NULL;
	}

	return ret;
}

static int localplayer_pause_handler(void)
{
	player_handler_t *handler = NULL;
	player_status_t status = PLAYER_UNKNOWN;

	pr_debug("...\n");
	handler = mozart_player_handler_get("player_saver", NULL, NULL);
	if (!handler) {
		pr_err("player handler get\n");
		return -1;
	}

	memset(&context, 0, sizeof(struct player_context));
	status = mozart_player_getstatus(handler);
	if (status == PLAYER_PLAYING) {
		if (mozart_localplayer_play_pause()) {
			pr_debug("localplayer play_pause fail\n");
			context = mozart_player_force_pause(handler);
		} else {
			if (mozart_player_wait_status(handler, PLAYER_PAUSED, 500 * 1000))
				context = mozart_player_force_pause(handler);
		}
	} else if (status == PLAYER_PAUSED) {
		pr_debug("player is paused\n");
	} else {
		pr_debug("player is %d\n", status);
		context = mozart_player_force_pause(handler);
	}

	mozart_player_handler_put(handler);

	return 0;
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int atalk_localplayer_module_start(struct mozart_module_struct *self)
{
	if (self->module_change) {
		mozart_smartui_boot_local();
		__mozart_prompt_tone_key_sync("atalk_local_4");
	}

	create_localplayer_start_play();
	self->player_state = player_state_idle;

	if (self->module_change)
		mozart_smartui_atalk_play("离线模式", "(没有本地歌曲)", "", "音乐收藏");

	return 0;
}

static int atalk_localplayer_module_run(struct mozart_module_struct * self)
{
	if (self->player_state == player_state_play)
		return localplayer_resume_handler();
	else
		return 0;
}

static int atalk_localplayer_module_suspend(struct mozart_module_struct * self)
{
	if (self->player_state == player_state_play)
		return localplayer_pause_handler();
	else
		return 0;
}

static int atalk_localplayer_module_stop(struct mozart_module_struct *self)
{
	player_handler_t *handler = NULL;

	handler = mozart_player_handler_get("player_saver", NULL, NULL);
	if (!handler) {
		pr_err("player handler get\n");
		return -1;
	}

	mozart_localplayer_stop_playback();
	if (mozart_player_wait_status(handler, PLAYER_STOPPED, 500 * 1000))
		mozart_player_force_stop(handler);

	mozart_player_handler_put(handler);

	mozart_smartui_atalk_toggle(false);
	self->player_state = player_state_idle;

	return 0;
}

static void atalk_localplayer_module_previous_song(struct mozart_module_struct *self)
{
	mozart_localplayer_prev_music();
}

static void atalk_localplayer_module_next_song(struct mozart_module_struct *self)
{
	mozart_localplayer_next_music();
}

static void atalk_localplayer_module_resume_pause(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (self->player_state == player_state_play) {
		/* pause */
		if (__mozart_module_is_run(self)) {
			localplayer_pause_handler();
			mozart_smartui_atalk_toggle(false);
			self->player_state = player_state_pause;
		} else if (__mozart_module_is_start(self)) {
			mozart_smartui_atalk_toggle(false);
			self->player_state = player_state_pause;
		}
	} else if (self->player_state == player_state_pause) {
		/* resume */
		if (__mozart_module_is_run(self)) {
			localplayer_resume_handler();
			mozart_smartui_atalk_toggle(true);
			self->player_state = player_state_play;
		} else if (__mozart_module_is_start(self)) {
			mozart_smartui_atalk_toggle(true);
			self->player_state = player_state_play;
		}
	}

	mozart_module_mutex_unlock();
}

static void atalk_localplayer_module_asr_wakeup(struct mozart_module_struct *self)
{
	mozart_smartui_asr_offline();
	mozart_prompt_tone_key_sync("atalk_asr_offline_23", true);
	mozart_smartui_asr_over();
}

static void atalk_localplayer_module_next_module(struct mozart_module_struct *self)
{
	mozart_bt_avk_start(false);
}

static struct mozart_module_struct atalk_localplayer_module = {
	.name = "atalk_localplayer",
	.priority = 1,
	.attach = module_attach,
	.mops = {
		.on_start   = atalk_localplayer_module_start,
		.on_run     = atalk_localplayer_module_run,
		.on_suspend = atalk_localplayer_module_suspend,
		.on_stop    = atalk_localplayer_module_stop,
	},
	.kops = {
		.previous_song = atalk_localplayer_module_previous_song,
		.next_song = atalk_localplayer_module_next_song,
		.resume_pause = atalk_localplayer_module_resume_pause,
		.asr_wakeup = atalk_localplayer_module_asr_wakeup,
		.next_module = atalk_localplayer_module_next_module,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_atalk_localplayer_is_start(void)
{
	return __mozart_module_is_start(&atalk_localplayer_module);
}

int mozart_atalk_localplayer_start(bool in_lock)
{
	if (atalk_localplayer_module.start) {
		return atalk_localplayer_module.start(&atalk_localplayer_module, module_cmd_stop, in_lock);
	} else {
		pr_err("atalk_localplayer_module isn't registered!\n");
		return -1;
	}
}

int mozart_atalk_localplayer_do_play(void)
{
	int i, ret;
	module_status domain_status;
	struct mozart_module_struct *self = &atalk_localplayer_module;

	/* wait 0.5s */
	for (i = 0; i < 10; i++) {
		if (share_mem_get(LOCALPLAYER_DOMAIN, &domain_status)) {
			pr_err("share_mem_get failed!\n");
			return -1;
		}

		if (domain_status != WAIT_RESPONSE)
			usleep(50 * 1000);
		else
			break;
	}
	if (i >= 10)
		pr_err("wait WAIT_REPONSE timeout!\n");

	mozart_module_mutex_lock();
	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_play;
		mozart_smartui_atalk_toggle(true);
		share_mem_set(LOCALPLAYER_DOMAIN, RESPONSE_DONE);
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_play;
		mozart_smartui_atalk_toggle(true);
		share_mem_set(LOCALPLAYER_DOMAIN, RESPONSE_PAUSE);
	} else {
		ret = -1;
		share_mem_set(LOCALPLAYER_DOMAIN, RESPONSE_CANCEL);
		mozart_module_mutex_unlock();
	}
	mozart_module_mutex_unlock();

	return ret;
}

int mozart_atalk_localplayer_startup(void)
{
	if (mozart_module_register(&atalk_localplayer_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
	mozart_localplayer_startup();

	return 0;
}

int mozart_atalk_localplayer_shutdown(void)
{
	if (atalk_localplayer_module.stop)
		atalk_localplayer_module.stop(&atalk_localplayer_module, module_cmd_stop, false);
	mozart_module_unregister(&atalk_localplayer_module);
	mozart_localplayer_shutdown();

	return 0;
}
