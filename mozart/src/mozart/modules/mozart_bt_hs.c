#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sharememory_interface.h"
#include "bluetooth_interface.h"
#include "volume_interface.h"
#include "ini_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_linein.h"

#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#include "vr-atalk_interface.h"
#include "mozart_atalk_cloudplayer_control.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"
#endif

#ifndef MOZART_RELEASE
#define MOZART_BT_HS_DEBUG
#endif

#ifdef MOZART_BT_HS_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[BT_HS] %s: "fmt, __func__, ##args)
#else  /* MOZART_BT_HS_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_BT_HS_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[BT_HS] [Error] %s: "fmt, __func__, ##args)

/*******************************************************************************
 * module
 *******************************************************************************/
static void bt_hs_set_volume(bool set)
{
	int volume = 30;
	char buf[32] = {};

	if (set) {
#if 1
		if (mozart_ini_getkey("/usr/data/system.ini", "volume", "bt", buf))
			printf("failed to parse /usr/data/system.ini, set BT music volume to 30.\n");
		else
			volume = atoi(buf);
#else
		volume = 60;
#endif

		mozart_volume_set(volume, BT_VOLUME);
	} else {
		if (mozart_ini_getkey("/usr/data/system.ini", "volume", "music", buf))
			printf("failed to parse /usr/data/system.ini, set music volume to 30.\n");
		else
			volume = atoi(buf);

		mozart_volume_set(volume, MUSIC_VOLUME);
	}
}

static int bt_hs_module_start(struct mozart_module_struct *self)
{
	mozart_smartui_bt_hs();

	return 0;
}

static int bt_hs_module_run(struct mozart_module_struct *self)
{
	bt_hs_set_volume(true);
	return 0;
}

static int bt_hs_module_suspend(struct mozart_module_struct *self)
{
	bt_hs_set_volume(false);
	return 0;
}

static int bt_hs_module_stop(struct mozart_module_struct *self)
{
	bt_hs_set_volume(false);
	return 0;
}

static void bt_hs_module_volume_up(struct mozart_module_struct *self)
{
}

static void bt_hs_module_volume_down(struct mozart_module_struct *self)
{
}

static void bt_hs_module_wifi_config(struct mozart_module_struct *self)
{
	/* Don't support */
}

static void bt_hs_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (mozart_linein_is_in())
		mozart_linein_start(true);
	else if (__mozart_module_is_online()){
		#if (SUPPORT_VR == VR_ATALK)
			mozart_atalk_cloudplayer_start(true);
		#elif (SUPPORT_VR == VR_SPEECH)
			mozart_aitalk_cloudplayer_start(true);
		#endif

	}
	else{
		#if (SUPPORT_VR == VR_ATALK)
			mozart_atalk_cloudplayer_start(true);
		#elif (SUPPORT_VR == VR_SPEECH)
			mozart_aitalk_cloudplayer_start(true);
		#endif
	}

	mozart_module_mutex_unlock();
}

static void bt_hs_module_resume_pause(struct mozart_module_struct *self)
{
	int state = mozart_bluetooth_hs_get_call_state();

	if (state == CALLSETUP_STATE_INCOMMING_CALL) {
		pr_debug(">>>>>>>>>>>>> answer call>>>>>\n");
		mozart_bluetooth_hs_answer_call();
	} else if (state == CALL_STATE_LEAST_ONE_CALL_ONGOING ||
		   state == CALLSETUP_STATE_REMOTE_BEING_ALERTED_IN_OUTGOING_CALL) {
		pr_debug(">>>>>>>>>>>>> hang up>>>>>\n");
		mozart_bluetooth_hs_hangup();
		mozart_smartui_bt_hs_disconnect();
	}
}

static void bt_hs_module_disconnect_handler(struct mozart_module_struct *self)
{
	mozart_bluetooth_hs_hangup();
	mozart_smartui_bt_hs_disconnect();
}

static struct mozart_module_struct bt_hs_module = {
	.name = "bt_hs",
	.priority = 8,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = bt_hs_module_start,
		.on_run     = bt_hs_module_run,
		.on_suspend = bt_hs_module_suspend,
		.on_stop    = bt_hs_module_stop,
	},
	.kops = {
		.volume_up = bt_hs_module_volume_up,
		.volume_down = bt_hs_module_volume_down,
		.wifi_config = bt_hs_module_wifi_config,
		.next_module = bt_hs_module_next_module,
		.resume_pause = bt_hs_module_resume_pause,
		.disconnect_handler = bt_hs_module_disconnect_handler,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_bt_hs_is_start(void)
{
	return __mozart_module_is_start(&bt_hs_module);
}

int mozart_bt_hs_start(bool in_lock)
{
	int i;
	module_status domain_status;

	/* wait 0.5s */
	for (i = 0; i < 10; i++) {
		if (share_mem_get(BT_HS_DOMAIN, &domain_status)) {
			pr_err("share_mem_get failed!\n");
		}

		if (domain_status != WAIT_RESPONSE)
			usleep(50 * 1000);
		else
			break;
	}

	if (i >= 10)
		pr_err("wait WAIT_REPONSE timeout!\n");

	share_mem_set(BT_HS_DOMAIN, RESPONSE_DONE);

	if (bt_hs_module.start) {
		return bt_hs_module.start(&bt_hs_module, module_cmd_suspend, in_lock);
	} else {
		pr_err("bt_hs_module isn't registered!\n");
		return -1;
	}
}

int mozart_bt_hs_stop(bool in_lock)
{
	if (bt_hs_module.stop) {
		bt_hs_module.stop(&bt_hs_module, module_cmd_run, in_lock);
		return 0;
	} else {
		pr_err("bt_hs_module isn't registered!\n");
		return -1;
	}
}

int mozart_bt_hs_startup(void)
{
	if (mozart_module_register(&bt_hs_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	return 0;
}

int mozart_bt_hs_shutdown(void)
{
	mozart_module_unregister(&bt_hs_module);

	return 0;
}
