#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "utils_interface.h"
#include "wifi_interface.h"
#include "localplayer_interface.h"
#include "bluetooth_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
//#include "mozart_channel.h"

//#include "mozart_led.h"

#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"

#ifndef MOZART_RELEASE
#define MOZART_ATALK_DEBUG
static char *aitalk_network_state_str[] = {
	[network_config] = "network_config",
	[network_online] = "network_online",
	[network_offline] = "network_offline",
};
#endif

#ifdef MOZART_ATALK_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[ATALK_NET] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[ATALK_NET] [Error] %s: "fmt, __func__, ##args)

/*******************************************************************************
 * API
 *******************************************************************************/
static bool aitalk_network_change = false;
static enum atalk_network_state network_original;


static void mozart_aitalk_net_change_handler(bool online)
{
	bool is_online, is_net;

	pr_debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	mozart_module_mutex_lock();
	__mozart_module_dump_state_map();

	is_online = __mozart_module_is_online();
	is_net = __mozart_module_is_net();

	if (online && is_net) {
		pr_debug("is net!\n");
		goto ret;
	}

	if (online && !is_online)
		__mozart_module_set_online();
	else if (!online && is_online)
		__mozart_module_set_offline();

	if (!__mozart_module_is_attach()) {
		pr_debug("is unattach!\n");
		goto ret;
	}

	if (online && !is_online) {
		mozart_aitalk_start(true);
	//	aitalk_monitor_cancel();
#if SUPPORT_SMARTUI
		mozart_smartui_boot_welcome();
#endif
	//	mozart_prompt_tone_key_sync("atalk_hi_12", true);
	} else if (!online && is_online) {
		ai_set_enable(false);
		//mozart_aitalk_cloudplayer_shutdown();
	//	mozart_aitalk_localplayer_start(true);
	}

	__mozart_module_dump_state_map();
ret:
	mozart_module_mutex_unlock();
}

static void __mozart_aitalk_online_handler(enum atalk_network_state ori)
{
//	mozart_smartui_boot_build_display("音箱已联网");
//	mozart_prompt_tone_key_sync("atalk_wifi_link_success_11", true);
/*
	if (ori == network_config || ori == network_offline) {
		atalk_cloudplayer_send_wifi_state(wifi_end_ok);
	} else {
		atalk_cloudplayer_send_wifi_state(wifi_start);
		atalk_cloudplayer_send_wifi_state(wifi_end_ok);
	}	//*/
}

static void __mozart_aitalk_offline_handler(enum atalk_network_state ori)
{
	if (ori == network_config) {
		/* atalk_cloudplayer_send_wifi_state(wifi_end_fail); */
	} else {
	//	atalk_cloudplayer_send_wifi_state(wifi_start);
		/* atalk_cloudplayer_send_wifi_state(wifi_end_fail); */
	}
}

enum atalk_network_state __mozart_aitalk_network_state(void)
{
	if (__mozart_module_is_net())
		return network_config;
	else if (__mozart_module_is_online())
		return network_online;
	else
		return network_offline;
}

int __mozart_aitalk_network_trigger(enum atalk_network_state cur, enum atalk_network_state ori, bool force)
{
	pr_debug("**** cur: %s, ori: %s ****\n", aitalk_network_state_str[cur], aitalk_network_state_str[ori]);

	if (!force && !__mozart_module_is_attach()) {
		pr_debug("module is unattach\n");
		if (!aitalk_network_change) {
			aitalk_network_change = true;
			network_original = ori;
		}

		return 0;
	}

	if (cur == ori)
		return 0;

	switch (cur) {
	case network_config:
		if (ori == network_online){
			//atalk_cloudplayer_send_wifi_state(wifi_start);
		}
		break;
	case network_online:
		mozart_aitalk_start(true);
		__mozart_aitalk_online_handler(ori);
		break;
	case network_offline:
	//	mozart_atalk_localplayer_start(true);
		//mozart_aitalk_cloudplayer_shutdown();
		ai_set_enable(false);
		__mozart_aitalk_offline_handler(ori);
		break;
	default:
		break;
	}

	return 0;
}

void mozart_aitalk_net_change(bool online)
{
	wifi_info_t infor = get_wifi_mode();

	if (online && infor.wifi_mode != STA) {
		pr_debug("net_change online, wifi_mode: %s\n", wifi_mode_str[infor.wifi_mode]);
		return ;
	}

	mozart_aitalk_net_change_handler(online);
}

int __mozart_aitalk_switch_mode(bool mode)
{
	int ret = 0;
	bool change;
	enum atalk_network_state network_state;

	pr_debug("PASS! %d\n",__LINE__);
	network_state = __mozart_aitalk_network_state();
	change = mode && aitalk_network_change;
	pr_debug("network_state: %s, change = %d\n", aitalk_network_state_str[network_state], change);

	if (change && network_state == network_online) {
		/* 在非阿里模式时, 发生过网络变化. */
		ret = -1;
	#if (SUPPORT_VR == VR_ATALK)
		atalk_vendor_shutdown();
	#elif (SUPPORT_VR == VR_SPEECH)
		aitalk_vendor_shutdown();
	#endif
	}

	pr_debug("PASS! %d\n",__LINE__);
	__aitalk_switch_mode(mode);

	if (change) {
		if (network_state == network_online) {
			__mozart_aitalk_online_handler(network_original);
		//	leds_set(LED_WHITE,255);
			mozart_aitalk_net_change(true);
			pr_debug("PASS! %d\n",__LINE__);
		#if (SUPPORT_VR == VR_ATALK)
			atalk_vendor_startup();
		#elif (SUPPORT_VR == VR_SPEECH)
			aitalk_vendor_startup();
		#endif
		} else if (network_state == network_offline) {
			__mozart_aitalk_offline_handler(network_original);
		}
	}
	aitalk_network_change = false;

	return ret;
}


void mozart_switch_aitalk_module(bool in_lock)
{
	pr_debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	if (!in_lock){
		mozart_module_mutex_lock();
	}
	//mozart_aitalk_start(true);
	if (__mozart_module_is_online()){
		mozart_aitalk_start(true);
	}
	else{
		ai_set_enable(false);
		//mozart_aitalk_cloudplayer_shutdown();//mozart_atalk_localplayer_start(true);
	}
//*/
	if (!in_lock)
		mozart_module_mutex_unlock();
}


