#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <execinfo.h>
#include <sys/time.h>

#include "battery_capacity.h"
#include "key_function.h"
#include "event_interface.h"
#include "wifi_interface.h"
#include "volume_interface.h"
#include "player_interface.h"
#include "tips_interface.h"
#include "app.h"
#include "airplay.h"
#include "linein.h"
#include "utils_interface.h"
#include "power_interface.h"
#include "sharememory_interface.h"
#include "ini_interface.h"
#include "mozart_config.h"
#include "alarm_interface.h"
#include "mozart_smartui.h"
#if (SUPPORT_VR != VR_NULL)
#include "vr.h"
#endif

//after mozart_config.h
#if (SUPPORT_LAPSULE == 1)
#include "lapsule_control.h"
#endif

bool key_mask = false;
bool battery_status;

event_handler *e_handler = NULL;
char *app_name = NULL;
int tfcard_status = -1;

int on_linein = 0;
pthread_mutex_t linein_lock = PTHREAD_MUTEX_INITIALIZER;
#if (SUPPORT_ATALK == 1)
int atalk_online_flag = 1;
pthread_mutex_t online_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static int from_net_config = 0;

static char *signal_str[] = {
	[1] = "SIGHUP",       [2] = "SIGINT",       [3] = "SIGQUIT",      [4] = "SIGILL",      [5] = "SIGTRAP",
	[6] = "SIGABRT",      [7] = "SIGBUS",       [8] = "SIGFPE",       [9] = "SIGKILL",     [10] = "SIGUSR1",
	[11] = "SIGSEGV",     [12] = "SIGUSR2",     [13] = "SIGPIPE",     [14] = "SIGALRM",    [15] = "SIGTERM",
	[16] = "SIGSTKFLT",   [17] = "SIGCHLD",     [18] = "SIGCONT",     [19] = "SIGSTOP",    [20] = "SIGTSTP",
	[21] = "SIGTTIN",     [22] = "SIGTTOU",     [23] = "SIGURG",      [24] = "SIGXCPU",    [25] = "SIGXFSZ",
	[26] = "SIGVTALRM",   [27] = "SIGPROF",     [28] = "SIGWINCH",    [29] = "SIGIO",      [30] = "SIGPWR",
	[31] = "SIGSYS",      [34] = "SIGRTMIN",    [35] = "SIGRTMIN+1",  [36] = "SIGRTMIN+2", [37] = "SIGRTMIN+3",
	[38] = "SIGRTMIN+4",  [39] = "SIGRTMIN+5",  [40] = "SIGRTMIN+6",  [41] = "SIGRTMIN+7", [42] = "SIGRTMIN+8",
	[43] = "SIGRTMIN+9",  [44] = "SIGRTMIN+10", [45] = "SIGRTMIN+11", [46] = "SIGRTMIN+12", [47] = "SIGRTMIN+13",
	[48] = "SIGRTMIN+14", [49] = "SIGRTMIN+15", [50] = "SIGRTMAX-14", [51] = "SIGRTMAX-13", [52] = "SIGRTMAX-12",
	[53] = "SIGRTMAX-11", [54] = "SIGRTMAX-10", [55] = "SIGRTMAX-9",  [56] = "SIGRTMAX-8", [57] = "SIGRTMAX-7",
	[58] = "SIGRTMAX-6",  [59] = "SIGRTMAX-5",  [60] = "SIGRTMAX-4",  [61] = "SIGRTMAX-3", [62] = "SIGRTMAX-2",
	[63] = "SIGRTMAX-1",  [64] = "SIGRTMAX",
};

static void usage(const char *app_name)
{
	printf("%s [-bsh] \n"
	       " -h     help (show this usage text)\n"
	       " -s/-S  TODO\n"
	       " -b/-B  run a daemon in the background\n", app_name);

	return;
}

char buf[16] = {};

void sig_handler(int signo)
{
	char cmd[64] = {};
	void *array[10];
	int size = 0;
	char **strings = NULL;
	int i = 0;

	printf("\n\n[%s: %d] mozart crashed by signal %s.\n", __func__, __LINE__, signal_str[signo]);

	printf("Call Trace:\n");
	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	if (strings) {
		for (i = 0; i < size; i++)
			printf ("  %s\n", strings[i]);
		free (strings);
	} else {
		printf("Not Found\n\n");
	}

	if (signo == SIGSEGV || signo == SIGBUS ||
	    signo == SIGTRAP || signo == SIGABRT) {
		sprintf(cmd, "cat /proc/%d/maps", getpid());
		printf("Process maps:\n");
		system(cmd);
	}

	printf("stop all services\n");
	stopall(-1);

	printf("unregister event manager\n");
	mozart_event_handler_put(e_handler);

	printf("unregister network manager\n");
	unregister_from_networkmanager();

	printf("unregister alarm manager\n");
	unregister_from_alarm_manager();

	share_mem_clear();
	share_mem_destory();

	exit(-1);
}

#if (SUPPORT_VR != VR_NULL)
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
int vr_flag = 0;
void *vr_longbutton_func(void *arg)
{
	vr_flag = 1;
	mozart_key_wakeup();
	vr_flag = 0;

	return NULL; // make compile happy.
}

int create_vr_longbutton_pthread()
{
	int ret = 0;
	pthread_t vr_longbutton_thread = 0;

	if(vr_flag == 0) {
		ret = pthread_create(&vr_longbutton_thread, NULL, vr_longbutton_func, NULL);
		if(ret != 0) {
			printf ("Create iflytek pthread failed: %s!\n", strerror(errno));
			return ret;
		}
		pthread_detach(vr_longbutton_thread);
	}

	return ret;
}
#endif
#endif

enum wifi_cmd wifi_config_cmd = SW_AP;
bool wifi_configing = false;
pthread_t wifi_config_pthread;
pthread_mutex_t wifi_config_lock = PTHREAD_MUTEX_INITIALIZER;

void *wifi_config_func(void *args)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	wifi_info_t infor = get_wifi_mode();

	wifi_configing = true;
	if (infor.wifi_mode == STA || infor.wifi_mode == STANET)
		wifi_config_cmd = SW_STA;
	else
		wifi_config_cmd = SW_AP;

	mozart_stop_snd_source_play();
	stopall(1);
	mozart_config_wifi();

	wifi_configing = false;

	return NULL; // make compile happy.
}

int create_wifi_config_pthread(void)
{
	pthread_mutex_lock(&wifi_config_lock);
	// cancle previous thread.
	if (wifi_configing)
		pthread_cancel(wifi_config_pthread);

	// enter wifi config mode.
	if (pthread_create(&wifi_config_pthread, NULL, wifi_config_func, NULL) == -1) {
		printf("Create wifi config pthread failed: %s.\n", strerror(errno));
		pthread_mutex_unlock(&wifi_config_lock);
		return -1;
	}
	pthread_detach(wifi_config_pthread);

	pthread_mutex_unlock(&wifi_config_lock);

	return 0;
}

bool wifi_switching = false;
pthread_t wifi_switch_pthread;
pthread_mutex_t wifi_switch_lock = PTHREAD_MUTEX_INITIALIZER;

void *wifi_switch_func(void *args)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	wifi_switching = true;

	mozart_stop_snd_source_play();
	stopall(1);
	mozart_wifi_mode();

	wifi_switching = false;

	return NULL; // make compile happy.
}

int create_wifi_switch_pthread(void)
{
	pthread_mutex_lock(&wifi_switch_lock);
	// cancle previous thread.
	if (wifi_switching)
		pthread_cancel(wifi_switch_pthread);

	// enter wifi switch mode.
	if (pthread_create(&wifi_switch_pthread, NULL, wifi_switch_func, NULL) == -1) {
		printf("Create wifi switch pthread failed: %s.\n", strerror(errno));
		pthread_mutex_unlock(&wifi_switch_lock);
		return -1;
	}
	pthread_detach(wifi_switch_pthread);

	pthread_mutex_unlock(&wifi_switch_lock);

	return 0;
}

#if (SUPPORT_BT == BT_BCM)
static pthread_mutex_t bt_switch_lock = PTHREAD_MUTEX_INITIALIZER;

void *bt_switch_func(void *args)
{
	mozart_stop();

	pthread_mutex_lock(&linein_lock);
	if (on_linein) {
		mozart_linein_off();
		on_linein = 0;
	}
	pthread_mutex_unlock(&linein_lock);

	if (!is_bt_mode()) {
		mozart_smartui_bt_play();
		bt_mode_set();
		snd_source = SND_SRC_BT_AVK;
	}

	mozart_bluetooth_avk_start_play();

	return NULL;
}

int create_bt_switch_pthread(void)
{
	pthread_t switch_pthread;

	pthread_mutex_lock(&bt_switch_lock);
	if (pthread_create(&switch_pthread, NULL, bt_switch_func, NULL) == -1) {
		printf("Create bt switch pthread failed: %s.\n", strerror(errno));
		pthread_mutex_unlock(&wifi_switch_lock);
		return -1;
	}
	pthread_detach(switch_pthread);
	pthread_mutex_unlock(&bt_switch_lock);

	return 0;
}
#endif

static int key_code = -1, key_code_last = -1, delay_us;
static bool key_combo_running;
static pthread_t key_combo_pthread;
static pthread_mutex_t key_combo_lock = PTHREAD_MUTEX_INITIALIZER;

static void key_handler(int key)
{
	switch (key) {
	case KEY_F1:
		if (key_mask)
			break;
		mozart_snd_source_switch();
		break;
	case KEY_F3:
		mozart_music_list(0);
		break;
	case KEY_F12:
		create_wifi_config_pthread(); // in case of block.
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
		mozart_factory_reset();
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
			printf("Create key scombo pthread failed: %s.\n", strerror(errno));
			pthread_mutex_unlock(&key_combo_lock);
			return ;
		}
		pthread_detach(key_combo_pthread);
	}

	pthread_mutex_unlock(&key_combo_lock);
}

extern bool mozart_is_asr;
extern int mozart_vr_atalk_interface_callback(void *arg);
void event_callback(mozart_event event, void *param)
{
	int state = 0;

	switch (event.type) {
	case EVENT_KEY:
		if (event.event.key.key.type != EV_KEY) {
			printf("Only support keyboard now.\n");
			break;
		}

		if (mozart_is_asr) {
			printf("ignore event_key\n");
			break;
		}

		if (event.event.key.key.value == 1) {
			switch (event.event.key.key.code)
			{
			case KEY_RECORD:
#if (SUPPORT_VR != VR_NULL)
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_SHORTPRESS || SUPPORT_VR_WAKEUP == VR_WAKEUP_VOICE_KEY_MIX)
				if (mozart_key_wakeup() == -2) {
					mozart_smartui_asr_offline();
					mozart_play_key((char *)"atalk_asr_offline_23");
				}
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
				if(create_vr_longbutton_pthread()) {  // in case of block.
					printf("create_iflytek_pthread failed!\n");
					break;
				}
#endif
#endif
				break;
			case KEY_MODE:
				create_wifi_switch_pthread(); // in case of block.
				break;
			case KEY_F1:
				create_combo_pthread(event.event.key.key.code,
						     event.event.key.key.value);
				break;
#if (SUPPORT_BT == BT_BCM)
			case KEY_BLUETOOTH:
				{
					state = mozart_bluetooth_hs_get_call_state();
					if (state == CALL_STATE_IS_COMMING) {
						printf(">>>>>>>>>>>>>answer call>>>>>\n");
						mozart_bluetooth_hs_answer_call();
					} else if (state == CALL_STATE_IS_ON_GOING) {
						printf(">>>>>>>>>>>>>hang up>>>>>\n");
						mozart_bluetooth_hs_hangup();
					}
				}
				break;
#endif
			case KEY_PREVIOUSSONG:
				mozart_previous_song();
				break;
			case KEY_NEXTSONG:
				mozart_next_song();
				break;
			case KEY_PLAYPAUSE:
#if (SUPPORT_BT == BT_BCM)
				switch (mozart_bluetooth_hs_get_call_state()) {
				case CALL_STATE_IS_COMMING:
					printf("Answer Call@BThs\n");
					mozart_bluetooth_hs_answer_call();
					break;
				case CALL_STATE_IS_ON_GOING:
					printf("Hang Up@BThs\n");
					mozart_bluetooth_hs_hangup();
					break;
				case CALL_STATE_NO_CALL_SETUP:
					mozart_play_pause();
					break;
				}
#else
				mozart_play_pause();
#endif
				break;
			case KEY_MENU:
				mozart_snd_source_switch();
				break;
			case KEY_VOLUMEUP:
				mozart_volume_up();
				break;
			case KEY_VOLUMEDOWN:
				mozart_volume_down();
				break;
			case KEY_F3:            /* music music Shortcut key 1 */
				create_combo_pthread(event.event.key.key.code,
						     event.event.key.key.value);
				break;
			case KEY_F4:            /* music music Shortcut key 2 */
				mozart_music_list(1);
				break;
			case KEY_F5:            /* music music Shortcut key 3 */
#if (SUPPORT_ATALK == 1)
				mozart_music_favorite();
#else
				mozart_music_list(2);
#endif
				break;
#if (SUPPORT_ATALK == 1)
			case KEY_F12:
				create_combo_pthread(event.event.key.key.code,
						     event.event.key.key.value);
				break;
#endif
			case KEY_POWER:
				mozart_power_off();
				break;
			default:
				//printf("UnSupport key down in %s:%s:%d.\n", __FILE__, __func__, __LINE__);
				break;
			}
		} else {
			switch (event.event.key.key.code) {
			case KEY_RECORD:
#if (SUPPORT_VR != VR_NULL)
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
				mozart_key_asr_record_end();
#endif
#endif
				break;
			default:
				break;
			}
		}
		printf("[DEBUG] key %s %s!!!\n",
		       keycode_str[event.event.key.key.code], keyvalue_str[event.event.key.key.value]);
		break;
	case EVENT_MISC:
		if (!strcasecmp(event.event.misc.name, "linein")) {
			printf("[device hotplug event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type, "plugin")) { // linein plugin
				mozart_smartui_linein_play();

				if (!mozart_atalk_is_control_mode()) {
					mozart_stop();
					mozart_play_key_sync("atalk_swich_false_5");
				} else {
					mozart_stop();
				}

				pthread_mutex_lock(&linein_lock);
				on_linein = 1;
				pthread_mutex_unlock(&linein_lock);
				mozart_linein_on();
#if (SUPPORT_LAPSULE == 1)
				lapsule_do_linein_on();
#endif
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // linein plugout
				pthread_mutex_lock(&linein_lock);
				if (on_linein) {
					mozart_linein_off();
					on_linein = 0;
#if(SUPPORT_LAPSULE == 1)
					lapsule_do_linein_off();
#endif
#if (SUPPORT_ATALK == 1)
					mozart_atalk_switch_mode();

					if (!atalk_online_flag) {
#if (SUPPORT_LOCALPLAYER == 1)
						mozart_smartui_boot_local();
						mozart_play_key_sync("atalk_local_4");

						share_mem_stop_other(LOCALPLAYER_DOMAIN);
						mozart_localplayer_start_playback();
						snd_source = SND_SRC_ATALK;
#endif
					}
#endif
				}
				pthread_mutex_unlock(&linein_lock);
			}
		} else if (!strcasecmp(event.event.misc.name, "tfcard")) {
			printf("[device hotplug event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
		} else if (!strcasecmp(event.event.misc.name, "headset")) {
			printf("[device hotplug event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type, "plugin")) { // headset plugin
				printf("headset plugin.\n");
				// do nothing.
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // headset plugout
				printf("headset plugout.\n");
				// do nothing.
			}
		} else if (!strcasecmp(event.event.misc.name, "spdif")) {
			printf("[device hotplug event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type, "plugin")) { // spdif-out plugin
				printf("spdif plugin.\n");
				// do nothing.
			} else if (!strcasecmp(event.event.misc.type, "plugout")) { // spdif-out plugout
				printf("spdif plugout.\n");
				// do nothing.
			}
#if (SUPPORT_BT == BT_BCM)
		} else if (!strcasecmp(event.event.misc.name, "bluetooth")) {
			printf("[software protocol event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type, "connected")) {
#if (SUPPORT_ATALK == 1)
				if (mozart_atalk_is_control_mode())
					mozart_atalk_switch_mode();
#endif

				create_bt_switch_pthread();
			} else if (!strcasecmp(event.event.misc.type, "disconnected")) {
#if (SUPPORT_ATALK == 1)
				if (is_bt_mode())
					mozart_atalk_switch_mode();
#endif
			} else if (!strcasecmp(event.event.misc.type, "connecting")) {
				// do nothing if bluetooth device is connecting.
			} else if (!strcasecmp(event.event.misc.type, "disc_complete")) {
				// do nothing if bluetooth device is finish discovery.
			} else if (!strcasecmp(event.event.misc.type, "play")) {
				if (mozart_smartui_is_bt_view())
					mozart_smartui_bt_toggle(true);
			} else if (!strcasecmp(event.event.misc.type, "pause")) {
				if (mozart_smartui_is_bt_view())
					mozart_smartui_bt_toggle(false);
			}
#endif
		} else if (!strcasecmp(event.event.misc.name, "dlna")) {
			printf("[software protocol event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type,"connected")) {
				// do nothing on dlna device connected event
			} else if (!strcasecmp(event.event.misc.type,"disconnected")) {
				// do nothing on dlna device connected event
			} else if (!strcasecmp(event.event.misc.type,"playurl")){
#if (SUPPORT_BT == BT_BCM)
				if (is_bt_mode()) {
					bt_mode_clear();
					/* Disconnect bluetooth, if bluetooth at linked state */
					if (mozart_bluetooth_get_link_status())
						mozart_bluetooth_disconnect();
				}
#endif
				pthread_mutex_lock(&linein_lock);
				if (on_linein) {
					mozart_linein_off();
					on_linein = 0;
				}
				pthread_mutex_unlock(&linein_lock);

#if (SUPPORT_ATALK == 1)
				if (mozart_atalk_is_control_mode())
					mozart_atalk_switch_mode();
#endif

				/* TODO: display dlna */
				mozart_smartui_linein_play();
			}

		} else if (!strcasecmp(event.event.misc.name, "airplay")) {
			printf("[software protocol event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
			if (!strcasecmp(event.event.misc.type,"connected")) {
#if (SUPPORT_BT == BT_BCM)
				if (is_bt_mode()) {
					bt_mode_clear();
					/* Disconnect bluetooth, if bluetooth at linked state */
					if (mozart_bluetooth_get_link_status())
						mozart_bluetooth_disconnect();
				}
#endif
				pthread_mutex_lock(&linein_lock);
				if (on_linein) {
					mozart_linein_off();
					on_linein = 0;
				}
				pthread_mutex_unlock(&linein_lock);

#if (SUPPORT_ATALK == 1)
				if (mozart_atalk_is_control_mode())
					mozart_atalk_switch_mode();
#endif

				/* TODO: display airplay */
				mozart_smartui_linein_play();

			} else if (!strcasecmp(event.event.misc.type,"disconnected")) {
				// do nothing on airplay device disconnected event
			}
#if (SUPPORT_ATALK == 1)
		} else if (!strcasecmp(event.event.misc.name, "atalk")) {
			if (!strcasecmp(event.event.misc.type,"prepare_play")) {
				share_mem_stop_other(ATALK_DOMAIN);
				if (is_bt_mode()) {
					bt_mode_clear();
					/* Disconnect bluetooth, if bluetooth at linked state */
					if (mozart_bluetooth_get_link_status())
						mozart_bluetooth_disconnect();
				}

				pthread_mutex_lock(&linein_lock);
				if (on_linein) {
					mozart_linein_off();
					on_linein = 0;
				}
				pthread_mutex_unlock(&linein_lock);
			} else if (!strcasecmp(event.event.misc.type, "online")) {
				pthread_mutex_lock(&online_lock);
				if (!atalk_online_flag && mozart_atalk_is_online()) {
					atalk_online_flag = 1;
				}
				pthread_mutex_unlock(&online_lock);
			} else if (!strcasecmp(event.event.misc.type, "offline")) {
				pthread_mutex_lock(&online_lock);
				if (atalk_online_flag) {
					atalk_online_flag = 0;
#if (SUPPORT_LOCALPLAYER == 1)
					if (mozart_atalk_is_control_mode()) {
						int ret = -1;
						memory_domain domain;

						mozart_smartui_boot_local();

						ret = share_mem_get_active_domain(&domain);
						if (!ret) {
							if (domain == ATALK_DOMAIN)
								mozart_atalk_stop_play();
							else
								share_mem_stop_other(LOCALPLAYER_DOMAIN);
						}
						mozart_localplayer_start_playback();
						snd_source = SND_SRC_ATALK;
					}
#endif
				}
				pthread_mutex_unlock(&online_lock);
			}
#endif
		} else if (!strcasecmp(event.event.misc.name,"vr")){
			if (!strcasecmp(event.event.misc.type,"vr wake up")) {
				mozart_smartui_asr_start();
			} else if (!strcasecmp(event.event.misc.type,"vr failed -1")) {
				mozart_smartui_asr_fail(1);
				mozart_play_key_sync((char *)"atalk_asr_again_26");
			} else if (!strcasecmp(event.event.misc.type,"vr failed -2")) {
				mozart_smartui_asr_fail(2);
				mozart_play_key_sync((char *)"atalk_asr_server_fail_28");
			} else if (!strcasecmp(event.event.misc.type,"vr failed -3")) {
				mozart_smartui_asr_fail(3);
				mozart_play_key_sync((char *)"atalk_asr_net_fail_29");
			} else if (!strcasecmp(event.event.misc.type,"vr success")) {
				mozart_smartui_asr_success();
			} else if (!strcasecmp(event.event.misc.type,"vr over")) {
#if (SUPPORT_ATALK == 1)
				mozart_vr_atalk_interface_callback((void *)ASR_STATE_IDLE);
#endif
			}
		} else if (!strcasecmp(event.event.misc.name,"localplayer")){
			if (!strcasecmp(event.event.misc.type,"play")) {
				char *title = NULL, *artist = NULL, *url = NULL;
				char *local_url = mozart_localplayer_get_current_url();

				if (local_url) {
					char *p;
					char *separator = "+++";

					url = local_url;
					while ((p = strstr(url, "/")) != NULL)
						url = p + 1;

					artist = strstr(url, separator);
					if (artist) {
						*artist = '\0';
						title = url;
						artist += strlen(separator);
					}
				}
				mozart_smartui_atalk_play("离线模式", title, artist, "音乐收藏");
				if (local_url)
					free(local_url);
			} else if (!strcasecmp(event.event.misc.type,"resume")) {
				mozart_smartui_atalk_toggle(true);
			} else if (!strcasecmp(event.event.misc.type,"pause")) {
				mozart_smartui_atalk_toggle(false);
			} else if (!strcasecmp(event.event.misc.type,"null")) {
				mozart_smartui_atalk_play("离线模式", "空", "", "音乐收藏");
			}
		} else if (!strcasecmp(event.event.misc.name,"volume")){
			if (!strcasecmp(event.event.misc.type,"update"))
				mozart_smartui_volume_update();
		} else if (!strcasecmp(event.event.misc.name,"charger")) {
			if (!strcasecmp(event.event.misc.type,"charged"))
				battery_status = true;
			else if (!strcasecmp(event.event.misc.type,"charging"))
				battery_status = false;
			mozart_smartui_battery_update();
		} else {
			printf("[misc event] %s : %s.\n", event.event.misc.name, event.event.misc.type);
		}
		break;
	default:
		break;
	}

	return;
}

int network_callback(const char *p)
{
	wifi_ctl_msg_t new_mode;
	event_info_t network_event;

	printf("\n[%s: %d] network event: %s\n\n", __func__, __LINE__, p);

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));
	memset(&network_event, 0, sizeof(event_info_t));
	network_event = parse_network_event_info((char *)p);

#if 0
	printf("********************************\n");
	printf("Event name:%s\n",network_event.name);
	printf("Event type:%s\n",network_event.type);
	printf("Event content:%s\n",network_event.content);
	printf("********************************\n");
#endif

	if(!strncmp(network_event.type, "NETWORK_CONFIGURE", strlen("NETWORK_CONFIGURE"))) {
		if(!strncmp(network_event.content, "AIRKISS_STARTING", strlen("AIRKISS_STARTING"))) {
			mozart_system("killall ntpd");

			key_mask = true;
#if (SUPPORT_BT == BT_BCM)
			if (is_bt_mode()) {
				bt_mode_clear();
				/* Disconnect bluetooth, if bluetooth at linked state */
				if (mozart_bluetooth_get_link_status())
					mozart_bluetooth_disconnect();
			}
#endif

			pthread_mutex_lock(&linein_lock);
			if (on_linein) {
				mozart_linein_off();
				on_linein = 0;
			}
			pthread_mutex_unlock(&linein_lock);

			from_net_config = 1;

			mozart_smartui_net_start();
#if (SUPPORT_ATALK == 1)
			atalk_network_trigger(true);
#endif
			mozart_play_key("atalk_wifi_config_7");
		} else if(!strncmp(network_event.content, "AIRKISS_SUCCESS", strlen("AIRKISS_SUCCESS"))) {
			new_mode.cmd = SW_STA;
			if (!request_wifi_mode(new_mode))
				printf("[Warning] %s: request_wifi_mode fail!\n", __func__);

			mozart_smartui_net_success();
			mozart_smartui_boot_link();
			mozart_play_key("atalk_wifi_config_success_8");
			key_mask = false;
		} else if(!strncmp(network_event.content, "AIRKISS_FAILED", strlen("AIRKISS_FAILED"))) {
			new_mode.cmd = wifi_config_cmd;
			if (!request_wifi_mode(new_mode))
				printf("[Warning] %s: request_wifi_mode fail!\n", __func__);

			mozart_smartui_net_fail();
			mozart_play_key_sync("atalk_wifi_config_fail_9");
			key_mask = false;
		} else if(!strncmp(network_event.content, "AIRKISS_CANCEL", strlen("AIRKISS_CANCEL"))) {
			new_mode.cmd = wifi_config_cmd;
			if (!request_wifi_mode(new_mode))
				printf("[Warning] %s: request_wifi_mode fail!\n", __func__);

			mozart_smartui_net_fail();
			mozart_play_key_sync("atalk_wifi_config_fail_9");
			/* mozart_play_key("airkiss_config_quit"); */
			key_mask = false;
		}
	} else if(!strncmp(network_event.type, "MODE_CHANGES", strlen("MODE_CHANGES"))) {
		wifi_info_t infor = get_wifi_mode();
		if (infor.wifi_mode == AP) {
#if (SUPPORT_ATALK == 1)
			atalk_network_trigger(true);
#endif
			startall(1);

#if (SUPPORT_ATALK == 1)
			pthread_mutex_lock(&online_lock);
			if (atalk_online_flag || from_net_config) {
				atalk_online_flag = 0;

				if (from_net_config) {
					if (!mozart_atalk_is_control_mode())
						mozart_atalk_switch_mode();
					from_net_config = 0;
				}

				if (mozart_atalk_is_control_mode()) {
					mozart_smartui_boot_local();
					mozart_play_key_sync("atalk_local_4");

					share_mem_stop_other(LOCALPLAYER_DOMAIN);
#if (SUPPORT_LOCALPLAYER == 1)
					mozart_localplayer_start_playback();
					snd_source = SND_SRC_ATALK;
#endif
				}
			}
			pthread_mutex_unlock(&online_lock);
#endif
		} else if (infor.wifi_mode == STA) {
			if (mozart_smartui_is_boot_view())
				mozart_smartui_boot_linked();
			mozart_play_key("atalk_wifi_link_success_11");

#if (SUPPORT_ATALK == 1)
			atalk_network_trigger(true);
#endif
			// XXX
			// 1. sync network time(and update system time with CST format)
			// 2. write it to hardware RTC(with UTC format).
			mozart_system("ntpd -nq");
			startall(1);

#if (SUPPORT_ATALK == 1)
			pthread_mutex_lock(&online_lock);
			if (!atalk_online_flag) {
				atalk_online_flag = 1;
			}
			pthread_mutex_unlock(&online_lock);

			if (from_net_config) {
				if (!mozart_atalk_is_control_mode())
					mozart_atalk_switch_mode();
				from_net_config = 0;
			}
#endif
		}
	} else if (!strncmp(network_event.type, "STA_STATUS", strlen("STA_STATUS"))) {
		mozart_system("killall ntpd");
	} else if(!strncmp(network_event.type, "STA_TIMEOUT", strlen("STA_TIMEOUT"))) {
		if (from_net_config) {
			new_mode.cmd = SW_AP;
			if (!request_wifi_mode(new_mode))
				printf("[Warning] %s: request_wifi_mode fail!\n", __func__);

			mozart_smartui_net_fail();
			mozart_play_key_sync("atalk_wifi_config_fail_9");
		} else {
			wifi_config_func(NULL);
			mozart_play_key_sync("atalk_offline_2");
		}
		stopall(1);
	} else if(!strncmp(network_event.type, "IDLE_MODE", strlen("IDLE_MODE"))) {
		/* TODO */
	}

	return 0;
}

int alarm_callback(const char *p , Alarm a)
{

	printf("alarm callback to do %s \n",p);
	printf("alarm after send message ====> id: %d,  hour: %d, minute: %d,  timestamp: %ld url is:%s\n",
	       a.alarm_id,  a.hour, a.minute,
	       a.timestamp, a.programUrl);

	return 0;
}

void *module_switch_func(void *args)
{
	module_status status;

	while (1) {
		usleep(100000);

		if(share_mem_get(AIRPLAY_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
			printf("[%s:%s:%d] airplay will stop playback.\n", __FILE__, __func__, __LINE__);
			continue;
		}

#if (SUPPORT_DMR == 1)
		if(share_mem_get(RENDER_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
			if (mozart_render_stop_playback()) {
				printf("stop render playback error in %s:%s:%d, continue.\n",
				       __FILE__, __func__, __LINE__);
				continue;
			}
		}
#endif

#if (SUPPORT_LOCALPLAYER == 1)
		if(share_mem_get(LOCALPLAYER_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
			if (mozart_localplayer_stop_playback()) {
				printf("stop localplayer playback error in %s:%s:%d, continue.\n",
				       __FILE__, __func__, __LINE__);
				continue;
			}
		}
#endif

#if (SUPPORT_ATALK == 1)
		if(share_mem_get(ATALK_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
			if (mozart_atalk_stop()) {
				printf("stop atalk error in %s:%s:%d, continue.\n",
				       __FILE__, __func__, __LINE__);
				continue;
			}
		}
#endif

#if (SUPPORT_VR != VR_NULL)
		int ret = 0;
		if(share_mem_get(VR_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_STOPPING) {
#if (SUPPORT_VR == VR_BAIDU)
			ret = mozart_vr_stop();
#elif (SUPPORT_VR == VR_SPEECH)
			printf("TODO: vr speech stop music.\n");
			ret = 0;
#elif (SUPPORT_VR == VR_IFLYTEK)
			printf("TODO: vr iflytek stop music.\n");
			ret = 0;
#elif (SUPPORT_VR == VR_UNISOUND)
			printf("TODO: vr unisound stop music.\n");
			ret = 0;
#endif

			if (ret) {
				printf("stop vr playback error in %s:%s:%d, continue.\n",
				       __FILE__, __func__, __LINE__);
				continue;
			}
		}
#endif

#if (SUPPORT_BT != BT_NULL)
		int cnt = 0;

		if(share_mem_get(BT_HS_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_BT_HS_AUD_REQ) {
#if (SUPPORT_VR != VR_NULL)
#if (SUPPORT_VR == VR_BAIDU)
			if (mozart_vr_get_status()) {
				printf("invoking mozart_vr_shutdown()\n");
				mozart_vr_shutdown();
			}
#elif (SUPPORT_VR == VR_SPEECH)
			if (mozart_vr_speech_get_status()) {
				printf("invoking mozart_vr_speech_shutdown()\n");
				mozart_vr_speech_shutdown();
			}
#elif (SUPPORT_VR == VR_IFLYTEK)
			if (mozart_vr_iflytek_get_status()) {
				printf("invoking mozart_vr_iflytek_shutdown()\n");
				mozart_vr_iflytek_shutdown();
			}
#elif (SUPPORT_VR == VR_UNISOUND)
			printf("TODO: shutdown vr unisound service.\n");
#elif (SUPPORT_VR == VR_ATALK)
			if (mozart_vr_atalk_get_status()) {
				printf("invoking mozart_vr_atalk_shutdown()\n");
				mozart_vr_atalk_shutdown();
			}
#endif	/* SUPPORT_VR == VR_XXX */
#endif	/* SUPPORT_VR != VR_NULL */
			if(AUDIO_OSS == get_audio_type()){
				cnt = 50;
				while (cnt--) {
					if (check_dsp_opt(O_WRONLY))
						break;
					usleep(100 * 1000);
				}
				if (cnt < 0) {
					printf("5 seconds later, /dev/dsp is still in use(writing), try later!!\n");
					continue;
				}
			}
#if (SUPPORT_VR != VR_NULL)
			if(AUDIO_OSS == get_audio_type()){
				cnt = 30;
				while (cnt--) {
					if (check_dsp_opt(O_RDONLY))
						break;
					usleep(100 * 1000);
				}
				if (cnt < 0) {
					printf("3 seconds later, /dev/dsp is still in use(reading), try later!!\n");
					continue;
				}
			}
#endif

			printf("You can answer the phone now.\n");
			//ensure dsp write _AND_ read is idle in here.
			if(share_mem_set(BT_HS_DOMAIN, STATUS_BT_HS_AUD_RDY))
				printf("share_mem_set failure.\n");
		}
#if (SUPPORT_VR != VR_NULL)
		else if (status == STATUS_RUNNING) {

			int wakeup_mode_mark = 0;
#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_VOICE)
			wakeup_mode_mark = VOICE_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_SHORTPRESS)
			wakeup_mode_mark = KEY_SHORTPRESS_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
			wakeup_mode_mark = KEY_LONGPRESS_WAKEUP;
#endif

#if (SUPPORT_VR == VR_BAIDU)
			if(!mozart_vr_get_status()) {
				printf("invoking mozart_vr_startup()\n");
				mozart_vr_startup();
			}
#elif (SUPPORT_VR == VR_SPEECH)
			if(!mozart_vr_speech_get_status()) {
				printf("invoking mozart_vr_speech_startup()\n");
				mozart_vr_speech_startup(wakeup_mode_mark, mozart_vr_speech_interface_callback);
			}
#elif (SUPPORT_VR == VR_IFLYTEK)
			if(!mozart_vr_iflytek_get_status()) {
				printf("invoking mozart_vr_iflytek_startup()\n");
				mozart_vr_iflytek_startup(wakeup_mode_mark, mozart_vr_iflytek_interface_callback);
			}
#elif (SUPPORT_VR == VR_UNISOUND)
			printf("TODO: restart vr unisound service.\n");
#elif (SUPPORT_VR == VR_ATALK)
			if (!mozart_vr_atalk_get_status()) {
				printf("invoking mozart_vr_atalk_startup()\n");
				mozart_vr_atalk_startup(wakeup_mode_mark, mozart_vr_atalk_interface_callback);
			}
#endif	/* SUPPORT_VR == VR_XXX */
		}
#endif	/* SUPPORT_VR != VR_NULL */
#if (SUPPORT_BT == BT_BCM)
		if(share_mem_get(BT_AVK_DOMAIN, &status))
			printf("share_mem_get failure.\n");
		if (status == STATUS_BT_AVK_AUD_REQ) {
			if(AUDIO_OSS == get_audio_type()){
				cnt = 50;
				while (cnt--) {
					if (check_dsp_opt(O_WRONLY))
						break;
					usleep(100 * 1000);
				}
				if (cnt < 0) {
					printf("5 seconds later, /dev/dsp is still in use(writing), try later!!\n");
					continue;
				}
			}

			printf("You can play music now.\n");
			//ensure dsp write _AND_ read is idle in here.
			if(share_mem_set(BT_AVK_DOMAIN, STATUS_BT_AVK_AUD_RDY))
				printf("share_mem_set failure.\n");
		} else if (status == STATUS_STOPPING) {
			mozart_bluetooth_avk_stop_play();
		}
#endif
#endif
	}

	return NULL;
}

static void get_battery_status(void)
{
	FILE *fp;
	char buffer[32];

	fp = popen("cat /sys/kernel/uevent_report/uevent_report/uevent", "r");
	if (!fp) {
		battery_status = false;
	} else {
		fgets(buffer, 32, fp);
		fclose(fp);
	}

	if (strstr(buffer, "charging"))
		battery_status = false;
	else
		battery_status = true;
}

static int initall(void)
{
	// recover volume in S01system_init.sh

	//share memory init.
	if(0 != share_mem_init()){
		printf("share_mem_init failure.\n");
	}
	if(0 != share_mem_clear()){
		printf("share_mem_clear failure.\n");
	}

	if (mozart_path_is_mount("/mnt/sdcard")) {
		tfcard_status = 1;
		share_mem_set(SDCARD_DOMAIN, STATUS_INSERT);
	} else {
		share_mem_set(SDCARD_DOMAIN, STATUS_EXTRACT);
	}

	if (mozart_path_is_mount("/mnt/usb"))
		share_mem_set(UDISK_DOMAIN, STATUS_INSERT);
	else
		share_mem_set(UDISK_DOMAIN, STATUS_EXTRACT);

	// shairport ini_interface.hinit(do not depend network part)

	// TODO: render init if needed.

	// TODO: localplayer init if needed.

	// TODO: bt init if needed.

	// TODO: vr init if needed.

	// TODO: other init if needed.

	return 0;
}

#if 0
void check_mem(int line)
{
	return ;
	pid_t pid = getpid();
	time_t timep;
	struct tm *p;
	FILE *fp;

	char rss[8] = {};
	char cmd[256] = {};
	char path[256] = {};

	// get current time.
	time(&timep);
	p=gmtime(&timep);


	printf("capture %d's mem info to %s.\n", pid, path);

	// get rss
	fp = popen("/bin/ps wl | grep mozart | grep -v grep | tr -s ' ' | cut -d' ' -f6", "r");
	if (!fp) {
		strcpy(rss, "99999");
	} else {
		fgets(rss, 5, fp);
		fclose(fp);
	}

	// capture exmap info.
	sprintf(path, "/tmp/%06d_%04d%02d%02d_%02d:%02d:%02d_%04d_%s.exmap",
		pid, (1900+p->tm_year), (1+p->tm_mon),p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, line, rss);

	sprintf(cmd, "echo %d > /proc/exmap; cat /proc/exmap > %s", pid, path);
	system(cmd);

	sprintf(cmd, "cat /proc/%d/maps >> %s", pid, path);

	system(cmd);
}
#endif

static void dump_compile_info(void)
{
	time_t timep;
	struct passwd *pwd;
	char hostname[16] = "Unknown";

	time(&timep);
	pwd = getpwuid(getuid());
	gethostname(hostname, 16);
	printf("mozart compiled at %s on %s@%s\n", asctime(gmtime(&timep)), pwd->pw_name, hostname);
}

int main(int argc, char **argv)
{
	int daemonize = 0;

	app_name = argv[0];
	struct wifi_client_register wifi_info;
	struct alarm_client_register alarm_info;
	pthread_t module_switch_thread;

	/* Get command line parameters */
	int c;
	while (1) {
		c = getopt(argc, argv, "bBsSh");
		if (c < 0)
			break;
		switch (c) {
		case 'b':
		case 'B':
			daemonize = 1;
			break;
		case 's':
		case 'S':
			break;
		case 'h':
			usage(app_name);
			return 0;
		default:
			usage(app_name);
			return 1;
		}
	}

	/* run in the background */
	if (daemonize) {
		if (daemon(0, 1)) {
			perror("daemon");
			return -1;
		}
	}
	dump_compile_info();

	initall();
	get_battery_status();

	int try_cnt = 0, ret;
	while (try_cnt++ < 6) {
		ret = mozart_play_key("atalk_welcome_1");
		if (!ret)
			break;
		usleep(500 * 1000);
	}
	mozart_smartui_startup();

	// start modules do not depend network.
	startall(0);

	/* wait for 5s if network is invalid now. */
	try_cnt = 0;
	while (try_cnt++ < 10) {
		if (!access("/var/run/wifi-server/register_socket", F_OK) &&
		    !access("/var/run/wifi-server/server_socket", F_OK))
			break;
		usleep(500 * 1000);
	}
	if (try_cnt >= 10)
		printf("[Warning] %s: Can't connect to networkmanager\n", __func__);

	// register network manager
	wifi_info.pid = getpid();
	wifi_info.reset = 1;
	wifi_info.priority = 3;
	strcpy(wifi_info.name, app_name);
	register_to_networkmanager(wifi_info, network_callback);

	wifi_info_t infor = get_wifi_mode();
	while (infor.wifi_mode == -1 && try_cnt++ < 10) {
		usleep(500 * 1000);
		printf("waiting network_manager ready in %s:%s:%d!!\n", __FILE__, __func__, __LINE__);
	}

	if (infor.wifi_mode != WIFI_NULL) {
		printf("[Warning] %s: wifi_mode != WIFI_NULL\n", __func__);
	} else {
		wifi_ctl_msg_t switch_sta;

		memset(&switch_sta, 0, sizeof(wifi_ctl_msg_t));
		switch_sta.cmd = SW_STA;
		if (!request_wifi_mode(switch_sta))
			printf("[Warning] %s: request_wifi_mode fail!\n", __func__);
	}

	register_battery_capacity();

	// register key event
	e_handler = mozart_event_handler_get(event_callback, app_name);

	//register alarm manager
	alarm_info.pid = getpid();
	alarm_info.reset = 1;
	alarm_info.priority = 3;
	strcpy(alarm_info.name, app_name);
	register_to_alarm_manager(alarm_info,alarm_callback);
	// switch render, airplay, localplayer, bt_audio, voice_recogition
	if (pthread_create(&module_switch_thread, NULL, module_switch_func, NULL) != 0) {
		printf("Can't create module_switch_thread in %s:%s:%d: %s\n",__FILE__, __func__, __LINE__, strerror(errno));
		exit(1);
	}
	pthread_detach(module_switch_thread);

	// process linein insert event on starup.
	if (mozart_linein_is_in()) {
		printf("linein detect, switch to linein mode...\n");
		mozart_linein_on();
	}

	// signal hander,
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	while(1)
		sleep(60);

	return 0;
}
