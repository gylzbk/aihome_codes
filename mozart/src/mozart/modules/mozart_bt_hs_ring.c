#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "sharememory_interface.h"
#include "bluetooth_interface.h"
#include "tips_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_linein.h"
#include "mozart_atalk.h"

#ifndef MOZART_RELEASE
#define MOZART_BT_HS_RING_DEBUG
#endif

#ifdef MOZART_BT_HS_RING_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[BT_HS_RING] %s: "fmt, __func__, ##args)
#else  /* MOZART_BT_HS_RING_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_BT_HS_RING_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[BT_HS_RING] [Error] %s: "fmt, __func__, ##args)

static int bt_ring;

static void *bt_hs_ring_play_prompt_func(void *data)
{
	pthread_detach(pthread_self());
	while (bt_ring == true)
		mozart_prompt_tone_key_sync("bt_call", false);
	return NULL;
}

static int bt_hs_ring_prompt(void)
{
	pthread_t prompt_thread;
	int err;

	if (bt_ring == true) {
		pr_err("bt_ring is running\n");
		return -1;
	}

	bt_ring = true;

	mozart_smartui_bt_hs();
	err = pthread_create(&prompt_thread, NULL, bt_hs_ring_play_prompt_func, NULL);
	if (err < 0) {
		pr_err("Create pthread: %s\n", strerror(errno));
		return -1;
	}
//	pthread_detach(prompt_thread);

	return 0;
}

static void bt_hs_ring_stop(void)
{
	if (bt_ring == true) {
		bt_ring = false;
		mozart_stop_tone_sync();
	}
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int bt_hs_ring_module_start(struct mozart_module_struct *self)
{
	return bt_hs_ring_prompt();
}

static int bt_hs_ring_module_run(struct mozart_module_struct *self)
{
	return 0;
}

static int bt_hs_ring_module_suspend(struct mozart_module_struct *self)
{
	return 0;
}

static int bt_hs_ring_module_stop(struct mozart_module_struct *self)
{
	bt_hs_ring_stop();

	return 0;
}

static void bt_hs_ring_module_wifi_config(struct mozart_module_struct *self)
{
	/* Don't support */
}

static void bt_hs_ring_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (mozart_linein_is_in())
		mozart_linein_start(true);
	else if (__mozart_module_is_online())
		mozart_atalk_cloudplayer_start(true);
	else
		mozart_atalk_cloudplayer_start(true);

	mozart_module_mutex_unlock();
}

static void bt_hs_ring_module_resume_pause(struct mozart_module_struct *self)
{
	int state = mozart_bluetooth_hs_get_call_state();

	if (state == CALLSETUP_STATE_INCOMMING_CALL) {
		pr_debug(">>>>>>>>>>>>>answer call>>>>>\n");
		mozart_bluetooth_hs_answer_call();
	}
}

static void bt_hs_ring_module_disconnect_handler(struct mozart_module_struct *self)
{
	mozart_bluetooth_hs_hangup();
	mozart_smartui_bt_hs_disconnect();
}

static struct mozart_module_struct bt_hs_ring_module = {
	.name = "bt_hs_ring",
	.priority = 8,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = bt_hs_ring_module_start,
		.on_run     = bt_hs_ring_module_run,
		.on_suspend = bt_hs_ring_module_suspend,
		.on_stop    = bt_hs_ring_module_stop,
	},
	.kops = {
		.wifi_config = bt_hs_ring_module_wifi_config,
		.next_module = bt_hs_ring_module_next_module,
		.resume_pause = bt_hs_ring_module_resume_pause,
		.disconnect_handler = bt_hs_ring_module_disconnect_handler,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_bt_hs_ring_is_start(void)
{
	return __mozart_module_is_start(&bt_hs_ring_module);
}

int mozart_bt_hs_ring_start(bool in_lock)
{
	if (bt_hs_ring_module.start) {
		return bt_hs_ring_module.start(&bt_hs_ring_module, module_cmd_suspend, in_lock);
	} else {
		pr_err("bt_hs_ring_module isn't registered!\n");
		return -1;
	}
}

int mozart_bt_hs_ring_stop(bool in_lock)
{
	bt_hs_ring_stop();

	if (bt_hs_ring_module.stop) {
		bt_hs_ring_module.stop(&bt_hs_ring_module, module_cmd_run, in_lock);
		return 0;
	} else {
		pr_err("bt_hs_ring_module isn't registered!\n");
		return -1;
	}
}

int mozart_bt_hs_ring_startup(void)
{
	if (mozart_module_register(&bt_hs_ring_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	return 0;
}

int mozart_bt_hs_ring_shutdown(void)
{
	mozart_module_unregister(&bt_hs_ring_module);

	return 0;
}
