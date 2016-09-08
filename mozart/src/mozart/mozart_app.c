#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "mozart_config.h"
#include "utils_interface.h"

#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_net.h"
#include "mozart_atalk.h"
#if (SUPPORT_VR == VR_SPEECH)
#include "mozart_speech_asr.h"
#endif
#include "mozart_dmr.h"
#include "mozart_airplay.h"
#include "mozart_bt_avk.h"
#include "mozart_bt_hs.h"
#include "mozart_bt_hs_ring.h"
#include "mozart_linein.h"
#include "mozart_battery.h"
#include "mozart_update_control.h"

#include "mozart_app.h"

static int stop_flag;

static void *time_depends_run_func(void *data)
{
	int err;

	while (!stop_flag) {
		err = mozart_system("pgrep ntpd > /dev/null");
		if (err == -1) {
			printf("[APP] [Error] %s. Run pgrep failed\n", __func__);
			return NULL;
		} else if (WEXITSTATUS(err)) {
			/* Not found ntpd pid means ntpd quit */
			break;
		}

		usleep(200 * 1000);
	}

	mozart_system("hwclock -w -f /dev/rtc0");

	/* Run Depend network time module
	 * after ntpd finished.
	 */
	/* Network time module run at here */
	mozart_update_control_startup();

	return NULL;
}

static int time_depends_start(void)
{
	pthread_t thread_time;
	int err;

	stop_flag = false;

	err = pthread_create(&thread_time, NULL, time_depends_run_func, NULL);
	if (err < 0) {
		printf("[APP] [Error] %s. Pthread create: %s\n", __func__, strerror(errno));
		return -1;
	}
	pthread_detach(thread_time);
	return 0;
}

static void mozart_startup_prompt(void)
{
	/* Display prompt ui */
	mozart_smartui_boot_start();
	/* Update battery ui */
	mozart_battery_update();
	/* Play prompt tone */
	mozart_prompt_tone_key_sync("atalk_welcome_1", true);
	/* Prompt Back from update, if needed */
	mozart_update_control_backfrom_update();
}

int startall(int depend_network)
{
	int wakeup_mode_mark = VOICE_KEY_MIX_WAKEUP;

#if (SUPPORT_VR_WAKEUP == VR_WAKEUP_VOICE)
	wakeup_mode_mark = VOICE_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_SHORTPRESS)
	wakeup_mode_mark = KEY_SHORTPRESS_WAKEUP;
#elif (SUPPORT_VR_WAKEUP == VR_WAKEUP_KEY_LONGPRESS)
	wakeup_mode_mark = KEY_LONGPRESS_WAKEUP;
#endif

	if (depend_network & APP_DEPEND_NO_NET) {
		mozart_smartui_startup();
		mozart_prompt_tone_startup();
		mozart_battery_startup();
		mozart_startup_prompt();

		mozart_net_startup();
		mozart_atalk_cloudplayer_startup();
		mozart_atalk_localplayer_startup();
#if 0
#if (SUPPORT_VR == VR_SPEECH)
		mozart_speech_asr_startup(wakeup_mode_mark);
#else
		mozart_atalk_asr_startup();
#endif
#endif

		mozart_bt_avk_startup();
		mozart_bt_hs_startup();
		mozart_bt_hs_ring_startup();
		mozart_linein_startup();
	}

	if (depend_network & APP_DEPEND_NET) {
		mozart_dmr_startup();
		mozart_airplay_startup();
		mozart_speech_asr_startup(wakeup_mode_mark);
	}

	if (depend_network & APP_DEPEND_NET_STA) {
		mozart_system("ntpd -nq &");
		mozart_system("dnsmasq &");
		time_depends_start();
	}

	return 0;
}

int stopall(int depend_network)
{
	if (depend_network & APP_DEPEND_NO_NET) {
		mozart_linein_shutdown();
		mozart_bt_hs_ring_shutdown();
		mozart_bt_hs_shutdown();
		mozart_bt_avk_shutdown();

		mozart_atalk_cloudplayer_shutdown();
		mozart_atalk_localplayer_shutdown();
#if 0
#if (SUPPORT_VR == VR_SPEECH)
		mozart_speech_asr_shutdown();
#else
		mozart_atalk_asr_shutdown();
#endif
#endif
		mozart_net_shutdown();

		mozart_battery_shutdown();
		mozart_prompt_tone_shutdown();
		mozart_smartui_shutdown();
	}

	if (depend_network & APP_DEPEND_NET) {
		mozart_dmr_shutdown();
		mozart_airplay_shutdown();
		mozart_speech_asr_shutdown();
	}

	if (depend_network & APP_DEPEND_NET_STA) {
		mozart_system("killall ntpd");
		mozart_system("killall dnsmasq > /dev/null 2>&1");
		mozart_update_control_shutdown();
		stop_flag = true;
	}

	return 0;
}
