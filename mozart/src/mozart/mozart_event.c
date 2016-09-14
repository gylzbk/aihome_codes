#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "localplayer_interface.h"
#include "wifi_interface.h"
#include "event_interface.h"
#include "bluetooth_interface.h"
#include "power_interface.h"
#include "utils_interface.h"
#include "tips_interface.h"
#include "ini_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_battery.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_event_str.h"
#include "mozart_dmr.h"
#include "mozart_airplay.h"
#include "mozart_bt_avk.h"
#include "mozart_bt_hs.h"
#include "mozart_bt_hs_ring.h"
#include "mozart_net.h"
#include "mozart_linein.h"
#include "mozart_update_control.h"

#include "mozart_key.h"
#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#include "vr-atalk_interface.h"
#include "mozart_atalk_cloudplayer_control.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"
#endif


#define MOZART_EVENT_DEBUG

#ifdef MOZART_EVENT_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[EVENT] %s: "fmt, __func__, ##args)
#else  /* MOZART_EVENT_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_EVENT_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[EVENT] [Error] %s: "fmt, __func__, ##args)

#define APP_HS_CALL_VOLUME_MAX  15
#define APP_AVK_VOLUME_MAX      17
/*******************************************************************************
 * key combo
 *******************************************************************************/
#if 0
static bool key_combo_running;
static pthread_t key_combo_pthread;
static pthread_mutex_t key_combo_lock = PTHREAD_MUTEX_INITIALIZER;
static int key_code = -1, key_code_last = -1, delay_us;

static void key_handler(int key)
{
	switch (key) {
	case KEY_F1:
		mozart_module_next_module();
		break;
	case KEY_F3:
		mozart_module_next_channel();
		break;
	case KEY_F12:
		mozart_module_wifi_config();
		break;
	default:
		break;
	}
}

static void *key_combo_func(void *args)
{
	int sum;

	usleep(delay_us);

	pthread_mutex_lock(&key_combo_lock);

	sum = key_code + key_code_last;
	if ((sum == (KEY_F12 + KEY_F3)) ||
	    (sum == (KEY_F1 + KEY_F3))) {
		mozart_module_factory_reset();
	} else {
		if ((key_code == KEY_F1 && key_code_last == KEY_F12) ||
		    (key_code == KEY_F12 && key_code_last == KEY_F1)) {
			key_handler(KEY_F12);
		} else {
			key_handler(key_code);
			key_handler(key_code_last);
		}
	}

	key_code_last = -1;
	key_combo_running = false;
	pthread_mutex_unlock(&key_combo_lock);

	return NULL;
}

static void create_combo_pthread(int key, int value)
{
	pthread_mutex_lock(&key_combo_lock);

	if (value == 1) {
		if (key_combo_running) {
			key_code_last = key;
			pthread_mutex_unlock(&key_combo_lock);
			return ;
		}

		key_code = key;
		key_combo_running = true;
		if (key == KEY_F3)	/* wait long press */
			delay_us = 700 * 1000;
		else
			delay_us = 200 * 1000;
		if (pthread_create(&key_combo_pthread, NULL, key_combo_func, NULL) == -1) {
			pr_err("Create key scombo pthread failed: %s.\n", strerror(errno));
			pthread_mutex_unlock(&key_combo_lock);
			return ;
		}
		pthread_detach(key_combo_pthread);
	}

	pthread_mutex_unlock(&key_combo_lock);
}
#endif

extern int mozart_ini_setkey(char *ini_file, char *section, char *key, char *value);
/*******************************************************************************
 * long press
 *******************************************************************************/
static void facotry_reset_handler(bool long_press)
{
	if (long_press) {
		mozart_stop_tone();
		mozart_module_factory_reset();
	}
}

static void playpause_handler(bool long_press)
{
	if (long_press)
		mozart_module_disconnect_handler();
	else
		mozart_module_resume_pause();
}

static void help_handler(bool long_press)
{
	if (long_press)
		mozart_system("/usr/fs/etc/init.d/S99adbd.sh start");
}

enum key_long_press_state {
	KEY_LONG_PRESS_INVALID = 0,
	KEY_LONG_PRESS_WAIT,
	KEY_LONG_PRESS_CANCEL,
	KEY_LONG_PRESS_DONE,
};
struct key_long_press_struct {
	char *name;
	enum key_long_press_state state;
	pthread_t pthread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int timeout_second;
	void (*handler)(bool long_press);
};

static struct key_long_press_struct facotry_reset_key_info = {
	.name = "facotry_reset_key",
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.cond = PTHREAD_COND_INITIALIZER,
	.timeout_second = 3,
	.handler = facotry_reset_handler,
};

static struct key_long_press_struct playpause_key_info = {
	.name = "playpause_key",
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.cond = PTHREAD_COND_INITIALIZER,
	.timeout_second = 3,
	.handler = playpause_handler,
};

static struct key_long_press_struct help_key_info = {
	.name = "help_key",
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.cond = PTHREAD_COND_INITIALIZER,
	.timeout_second = 3,
	.handler = help_handler,
};

static void *key_long_press_func(void *args)
{

	pthread_detach(pthread_self());
	struct timeval now;
	struct timespec timeout;
	struct key_long_press_struct *info = (struct key_long_press_struct *)args;

	pthread_mutex_lock(&info->lock);
	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + info->timeout_second;
	timeout.tv_nsec = now.tv_usec * 1000;

	pthread_cond_timedwait(&info->cond,
			       &info->lock, &timeout);

	if (info->state == KEY_LONG_PRESS_CANCEL) {
		info->state = KEY_LONG_PRESS_INVALID;
		pr_debug("%s short press\n", info->name);
		if (info->handler)
			info->handler(false);
		pthread_mutex_unlock(&info->lock);
		return NULL;
	}

	info->state = KEY_LONG_PRESS_DONE;

	pr_debug("%s long press\n", info->name);
	if (info->handler)
		info->handler(true);
	pthread_mutex_unlock(&info->lock);

	return NULL;
}

static void create_key_long_press_pthread(struct key_long_press_struct *info)
{
	pthread_mutex_lock(&info->lock);
	pr_debug("%s's state = %d\n", info->name, info->state);
	if (info->state != KEY_LONG_PRESS_INVALID) {
		pthread_mutex_unlock(&info->lock);
		return ;
	}
	info->state = KEY_LONG_PRESS_WAIT;
	pthread_mutex_unlock(&info->lock);

	if (pthread_create(&info->pthread, NULL, key_long_press_func, (void *)info) == -1) {
		pr_err("Create key long press pthread failed: %s.\n", strerror(errno));
		return ;
	}
//	pthread_detach(info->pthread);
}

static void key_long_press_cancel(struct key_long_press_struct *info)
{
	pthread_mutex_lock(&info->lock);

	if (info->state == KEY_LONG_PRESS_DONE) {
		info->state = KEY_LONG_PRESS_INVALID;
	} else if (info->state == KEY_LONG_PRESS_WAIT) {
		info->state = KEY_LONG_PRESS_CANCEL;
		pthread_cond_signal(&info->cond);
	}

	pthread_mutex_unlock(&info->lock);
}

/*******************************************************************************
 * event handler
 *******************************************************************************/
static void mozart_event_key(mozart_event event)
{
	int code = event.event.key.key.code;
	int value = event.event.key.key.value;

	if (event.event.key.key.type != EV_KEY) {
		printf("Only support keyboard now.\n");
		return ;
	}

	pr_debug("%s: %s\n", keycode_str[code], keyvalue_str[value]);

	if (event.event.key.key.value == 1) {
		if(code  == KEY_POWER){	//-------------- wifi config
			mozart_stop_tone();
			mozart_key_ignore_set(false);
			mozart_module_power_off("正在关机");
			return;
		}
		if(code  == KEY_F1){		//-------------- wifi
			mozart_stop_tone();
			mozart_key_ignore_set(false);
			mozart_module_wifi_config();
			return;
		}
		if(code  == KEY_MODE){		//-------------- mode
			mozart_stop_tone();
			mozart_module_next_module();
			return;
		}
		if(code  == KEY_RECORD){		//-------------- mode
			if(__mozart_aitalk_cloudplayer_is_asr()){
				mozart_aitalk_sem_stop();
			}
			return;
		}
	}

	if (mozart_key_ignore_get()){
		pr_debug("Error: key ignore !\n");
		return;
	}

	if (event.event.key.key.value == 1) {
		switch (code) {
		case KEY_RECORD:
		//	mozart_aitalk_asr_over();
		//	mozart_module_asr_wakeup();
			break;
		case KEY_PREVIOUSSONG:
			mozart_module_previous_song();
			break;
		case KEY_NEXTSONG:
			mozart_module_next_song();
			break;
		case KEY_PLAYPAUSE:
			create_key_long_press_pthread(&playpause_key_info);
			break;
		case KEY_VOLUMEUP:
			mozart_module_volume_up();
			break;
		case KEY_VOLUMEDOWN:
			mozart_module_volume_down();
			break;
		case KEY_MENU:
			mozart_module_favorite();
			break;
		case KEY_F3:
			mozart_module_next_channel();
			break;
		case KEY_F10:
			mozart_module_mutex_lock();
			if (__mozart_module_is_online())
				mozart_update_control_try_start(true);
			mozart_module_mutex_unlock();
			break;
		case KEY_F12:
			create_key_long_press_pthread(&facotry_reset_key_info);
			break;
		case KEY_HELP:
			create_key_long_press_pthread(&help_key_info);
			break;

		default:
			break;
		}
	} else {
		switch (code) {
		case KEY_F12:
			key_long_press_cancel(&facotry_reset_key_info);
			break;
		case KEY_PLAYPAUSE:
			key_long_press_cancel(&playpause_key_info);
			break;
		case KEY_HELP:
			key_long_press_cancel(&help_key_info);
			break;
		case KEY_RECORD:
			/* mozart_module_asr_cancel(); */
			break;
		default:
			break;
		}
	}
}

/* static int vr_fail_cnt; */
static void mozart_event_misc(mozart_event event)
{
	char *name = event.event.misc.name;
	char *type = event.event.misc.type;

	pr_debug("%s: %s\n", name, type);

	if (!strcasecmp(name, "volume")) {
		if (!strcasecmp(type, "update music")) {
			mozart_smartui_volume_update();
			mozart_module_volume_change();
		}
	} else if (!strcasecmp(name, "localplayer")) {
		if (!strcasecmp(type, "play")) {
			#if (SUPPORT_VR == VR_ATALK)
				mozart_atalk_localplayer_do_play();
			#elif (SUPPORT_VR == VR_SPEECH)
				mozart_aitalk_localplayer_do_play();
			#endif
		} else if (!strcasecmp(type, "playing")) {
			char *title = NULL, *artist = NULL, *url = NULL;
			char *local_url = mozart_localplayer_get_current_url();

			if (local_url) {
				char *p;
				char *separator = "+++";

				url = local_url;
				while ((p = strstr(url, "/")) != NULL)
					url = p + 1;

				title = strstr(url, separator);
				if (title) {
					*title = '\0';
					artist = url;
					title += strlen(separator);
				}
			}
			mozart_smartui_atalk_play("离线模式", title, artist, "音乐收藏");
			if (local_url)
				free(local_url);
		}
	} else if (!strcasecmp(name, "vr")) {
#if 0
		if (!strcasecmp(event.event.misc.type, "vr recognize")) {
			mozart_smartui_asr_recognize();
		} else if (!strcasecmp(event.event.misc.type, "vr failed -1")) {
			if (vr_fail_cnt++ >= 2) {
				vr_fail_cnt = 0;
				mozart_smartui_asr_fail("请用APP搜歌，我们会改进语音输入体验");
				mozart_prompt_tone_key_sync("atalk_asr_fail_27", false);
			} else {
				mozart_smartui_asr_fail("没听清请再试一次");
				mozart_prompt_tone_key_sync("atalk_asr_again_26", false);
			}
		} else if (!strcasecmp(event.event.misc.type, "vr failed -2")) {
			vr_fail_cnt = 0;
			mozart_smartui_asr_fail("服务忙，请稍候再试");
			mozart_prompt_tone_key_sync("atalk_asr_server_fail_28", false);
		} else if (!strcasecmp(event.event.misc.type, "vr failed -3")) {
			vr_fail_cnt = 0;
			mozart_smartui_asr_fail("网络有问题，请检查网络");
			mozart_prompt_tone_key_sync("atalk_asr_net_fail_29", false);
		} else if (!strcasecmp(event.event.misc.type, "vr success")) {
			vr_fail_cnt = 0;
			mozart_atalk_asr_over();
		} else if (!strcasecmp(event.event.misc.type, "vr over")) {
			mozart_atalk_asr_over();
		}
#endif
	} else if (!strcasecmp(name, "dlna")) {
		if (!strcasecmp(type, "play"))
			mozart_dmr_start(false);
		else if (!strcasecmp(type, "resume"))
			mozart_dmr_do_resume();
		else if (!strcasecmp(type, "pause"))
			mozart_dmr_do_pause();
	} else if (!strcasecmp(name, "airplay")) {
		if (!strcasecmp(type, "connected")) {
			mozart_airplay_start(false);
		} else if (!strcasecmp(type, "disconnected")) {
			mozart_module_mutex_lock();
			if (__mozart_airplay_is_start()){
				#if (SUPPORT_VR == VR_ATALK)
					mozart_switch_atalk_module(true);
				#elif (SUPPORT_VR == VR_SPEECH)
					mozart_switch_aitalk_module(true);
				#endif
			}
			mozart_module_mutex_unlock();
		}
	} else if (!strcasecmp(name, "bluetooth")) {
		if (!strcasecmp(type, "connecting"))
			mozart_smartui_bt_connecting();
	} else if (!strcasecmp(name, "bt_avk")) {
		if (!strcasecmp(type, "connected")) {
			mozart_module_mutex_lock();
			if (__mozart_bt_avk_is_start()) {
				mozart_smartui_bt_connected();
				mozart_prompt_tone_key_sync("bt_connected", true);
				mozart_module_mutex_unlock();
			} else {
				int ret, timeout = 100;

				mozart_module_mutex_unlock();
				do {
					ret = mozart_bluetooth_disconnect(USE_HS_AVK);
					usleep(100 * 1000);
					if (timeout-- <= 0)
						break;
				} while (ret);
			}
		} else if (!strcasecmp(type, "play")) {
			mozart_bt_avk_do_play();
		} else if (!strcasecmp(type, "pause")) {
			mozart_bt_avk_do_pause();
		} else if (!strcasecmp(type, "disconnected")) {
			mozart_module_mutex_lock();
			if (__mozart_bt_avk_is_start()) {
				mozart_smartui_bt_disconnect();
				mozart_prompt_tone_key_sync("bt_disconnect", true);
			}
			mozart_module_mutex_unlock();
		} else if (!strcasecmp(event.event.misc.type, "set_abs_vol")) {
				int index = 0;
				int volume = 0;
				int fd = 0;
				UINT8 avk_volume_set_dsp[APP_AVK_VOLUME_MAX] = {0, 6, 12, 18, 25, 31, 37, 43, 50, 56, 62, 68, 75, 81, 87, 93, 100};
				UINT8 avk_volume_set_phone[APP_AVK_VOLUME_MAX] = {0, 7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127};

				volume = event.event.misc.value[0];
				fd = event.event.misc.value[1];
				for (index = (APP_AVK_VOLUME_MAX -1); index >= 0; index--) {
					if (avk_volume_set_phone[index] <= volume)
						break;
				}
				if (index < 0) {
					printf("failed to get music volume %d from avk_volume_set_dsp\n", volume);
				}
				/* Note: add app_avk_cb.fd judged, because when play tts,
				 * we reviced ABS_VOL_CMD_EVT, and set volume to dsp,
				 * it will generate a case of sound mutation */
				if (fd != -1) {
					mozart_volume_set(avk_volume_set_dsp[index], MUSIC_VOLUME);
				} else {
					char vol[8] = {};
					sprintf(vol, "%d", avk_volume_set_dsp[index]);
					if (mozart_ini_setkey("/usr/data/system.ini", "volume", "music", vol)) {
						printf("save volume to /usr/data/system.ini error.\n");
					}
				}
				printf("phone volume: 0x%x, mozart set avk volume: %d\n",
					   volume,
					   avk_volume_set_dsp[index]);
		} else if (!strcasecmp(event.event.misc.type, "get_elem_attr")) {
				int i = 0;
				int attr_id = 0;
				tBSA_AVK_GET_ELEMENT_ATTR_MSG *p_data = NULL;
				p_data = mozart_bluetooth_avk_get_element_att();
				printf("p_data->num_attr = %d\n", p_data->num_attr);
				printf("p_data->status = %d\n", p_data->status);
				for (i = 0; i < p_data->num_attr; i++) {
					attr_id = p_data->attr_entry[i].attr_id;
					if (attr_id == AVRC_MEDIA_ATTR_ID_TITLE) {
						printf("music Title: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_ARTIST) {
						printf("music Artist Name: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_ALBUM) {
						printf("music Album Name: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_TRACK_NUM) {
						printf("music Track Number: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_NUM_TRACKS) {
						printf("music Total Number of Tracks: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_GENRE) {
						printf("music Genre: %s\n", p_data->attr_entry[i].name.data);
					} else if (attr_id == AVRC_MEDIA_ATTR_ID_PLAYING_TIME) {
						printf("music Playing Time: %s\n", p_data->attr_entry[i].name.data);
					}
				}
		}
	} else if (!strcasecmp(name, "bt_hs")) {
		/* Andriod call:
		 *    + 铃声: [ring...]
		 *    + 铃声时挂断: [ring...] -> hangup
		 *    + 接听: [ring...] -> call -> hangup
		 *    + 挂断: [ring...] -> call -> hangup -> close (state = 0)
		 *
		 * Iphone call:
		 *    + 按键接听: call -> [ring...] -> close -> answer_call() -> hangup
		 *    + 铃声时挂断: call -> [ring ...] -> hangup -> close
		 *    + 手机接听: call -> [ring...] -> close (state = 1) -> hangup (此时在手机端能听到声音)
		 *    + 手机选择音箱为音频设备: call
		 *    + 挂断: close (state = 0)
		 */
		pr_debug("hs call state: %d\n", mozart_bluetooth_hs_get_call_state());
		if (!strcasecmp(type, "ring")) {
			mozart_module_mutex_lock();
			if (!__mozart_bt_hs_is_start() && !__mozart_bt_hs_ring_is_start())
				mozart_bt_hs_ring_start(true);
			mozart_module_mutex_unlock();
		} else if (!strcasecmp(type, "hangup")) {
			mozart_module_mutex_lock();
			if (__mozart_bt_hs_ring_is_start()) {
				mozart_bt_hs_ring_stop(true);
				mozart_smartui_bt_hs_disconnect();
			}
			mozart_module_mutex_unlock();
		} else if (!strcasecmp(type, "call")) {
			mozart_module_mutex_lock();
			if (__mozart_bt_hs_ring_is_start())
				mozart_bt_hs_ring_stop(true);
			mozart_module_mutex_unlock();

			mozart_bt_hs_start(false);
		} else if (!strcasecmp(type, "close")) {
			int state = mozart_bluetooth_hs_get_call_state();

			/*
			 * iphone
			 * CALL_STATE_NO_CALLS_ONGOING: 来电中挂断
			 * CALL_STATE_NO_CALLS_ONGOING: 通话中挂断
			 * CALL_STATE_LEAST_ONE_CALL_ONGOING: 通话中, 放音设备选择手机
			 * CALLSETUP_STATE_NO_CALL_SETUP: 去电中挂断
			 * CALLSETUP_STATE_REMOTE_BEING_ALERTED_IN_OUTGOING_CALL: 去电中更换放音设备
			 */
			if (state == CALL_STATE_NO_CALLS_ONGOING ||
			    state == CALL_STATE_LEAST_ONE_CALL_ONGOING ||
			    state == CALLSETUP_STATE_NO_CALL_SETUP ||
			    state == CALLSETUP_STATE_REMOTE_BEING_ALERTED_IN_OUTGOING_CALL) {
				mozart_module_mutex_lock();
				if (__mozart_bt_hs_is_start()) {
					mozart_bt_hs_stop(true);
					mozart_smartui_bt_hs_disconnect();
				}
				mozart_module_mutex_unlock();
			}
		} else if (!strcasecmp(type, "disconnected")) {
			mozart_module_mutex_lock();
			if (__mozart_bt_hs_ring_is_start()) {
				mozart_bt_hs_ring_stop(true);
				mozart_smartui_bt_hs_disconnect();
			} else if (__mozart_bt_hs_is_start()) {
				mozart_bt_hs_stop(true);
				mozart_smartui_bt_hs_disconnect();
			}
			mozart_module_mutex_unlock();
		} else if (!strcasecmp(type, "vgs")) {
			int volume = event.event.misc.value[0];

			mozart_volume_set(volume * 100 / APP_HS_CALL_VOLUME_MAX, BT_VOLUME);
			printf("phone volume: %d, mozart set hs volume: %d\n",
					volume,
					volume * 100 / APP_HS_CALL_VOLUME_MAX);

		} else {
			printf("unhandle bt phone event: %s.\n", event.event.misc.type);
		}

	} else if (!strcasecmp(name, "bt_avrcp")) {
		if (!strcasecmp(event.event.misc.type, "playing")) {
			printf("playing!\n");
		} else if (!strcasecmp(event.event.misc.type, "paused")) {
			printf("pause!\n");
		} else if (!strcasecmp(event.event.misc.type, "stopped")) {
			printf("stopped!\n");
		} else if (!strcasecmp(event.event.misc.type, "track_change")) {
			mozart_bluetooth_avk_send_get_element_att_cmd();
		} else if (!strcasecmp(event.event.misc.type, "play_pos")) {
			printf("play_pos: %d\n", event.event.misc.value[0]);
		}
	} else if (!strcasecmp(name, "linein")) {
		if (!strcasecmp(type, "plugin") && mozart_linein_is_in()) {
			mozart_stop_tone();
			mozart_linein_start(false);
		} else if (!strcasecmp(type, "plugout") && !mozart_linein_is_in()) {
			mozart_module_mutex_lock();
			if (__mozart_linein_is_start()){
				#if (SUPPORT_VR == VR_ATALK)
					mozart_switch_atalk_module(true);
				#elif (SUPPORT_VR == VR_SPEECH)
					mozart_switch_aitalk_module(true);
				#endif
			}
			mozart_module_mutex_unlock();
		}
	}
}

void mozart_event_key_callback(mozart_event event, void *param)
{
	if (event.type == EVENT_KEY)
		mozart_event_key(event);
}
void mozart_event_misc_callback(mozart_event event, void *param)
{
	if (event.type == EVENT_MISC)
		mozart_event_misc(event);
}

void mozart_event_callback(mozart_event event, void *param)
{
	char *name = event.event.misc.name;
	char *type = event.event.misc.type;

	if (event.type == EVENT_MISC) {
		/* pr_debug("%s: %s\n", name, type); */

		if (!strcasecmp(name, "battery") && !strcasecmp(type, "change")) {
			mozart_battery_update();
		} else if (!strcasecmp(name, "bt_avk") && !strcasecmp(type, "disconnected")) {
			mozart_module_mutex_lock();
			if (!__mozart_bt_avk_is_start())
				mozart_bluetooth_set_visibility(false, false);
			mozart_module_mutex_unlock();
		}
	}
}
