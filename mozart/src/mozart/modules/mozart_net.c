#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <json-c/json.h>

#include "tips_interface.h"
#include "utils_interface.h"
#include "wifi_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_app.h"
#include "mozart_net.h"
#include "mozart_bt_avk.h"


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
#define MOZART_NET_DEBUG
#endif

#ifdef MOZART_NET_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[NET] %s: "fmt, __func__, ##args)
#else  /* MOZART_NET_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_NET_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[NET] [Error] %s: "fmt, __func__, ##args)

enum net_mode_enum {
	NET_MODE_INVALID = 0,
	NET_MODE_BOOT_STA,
	NET_MODE_SW_NETCFG,
	NET_MODE_CFG_START,
	NET_MODE_CFG_CANCEL,
	NET_MODE_CFG_STA,
	NET_MODE_SW_STA,
	NET_MODE_SW_STA_ALLTIME,
	NET_MODE_STA,
};

static char *net_mode_str[] = {
	[NET_MODE_INVALID] = "NET_MODE_INVALID",
	[NET_MODE_BOOT_STA] = "NET_MODE_BOOT_STA",
	[NET_MODE_SW_NETCFG] = "NET_MODE_SW_NETCFG",
	[NET_MODE_CFG_START] = "NET_MODE_CFG_START",
	[NET_MODE_CFG_CANCEL] = "NET_MODE_CFG_CANCEL",
	[NET_MODE_CFG_STA] = "NET_MODE_CFG_STA",
	[NET_MODE_SW_STA] = "NET_MODE_SW_STA",
	[NET_MODE_SW_STA_ALLTIME] = "NET_MODE_SW_STA_ALLTIME",
	[NET_MODE_STA] = "NET_MODE_STA",
};

static enum net_mode_enum global_net_mode;
static bool wrong_key;
static pthread_t net_config_pthread;
static pthread_mutex_t net_mode_lock = PTHREAD_MUTEX_INITIALIZER;

static bool stop_net_config(void);
static bool network_switch_sta_mode(int timeout);
/*******************************************************************************
 * module
 *******************************************************************************/
static int net_module_start(struct mozart_module_struct *current)
{
	return 0;
}

static int net_module_run(struct mozart_module_struct *current)
{
	return 0;
}

static int net_module_suspend(struct mozart_module_struct *current)
{
	return 0;
}

static int net_module_stop(struct mozart_module_struct *current)
{
	wifi_info_t infor = get_wifi_mode();

	module_mutex_lock(&net_mode_lock);

	pr_debug("net_mode is %s, infor.wifi_mode = %d\n\n",
		 net_mode_str[global_net_mode], infor.wifi_mode);

	if (global_net_mode == NET_MODE_CFG_START)
		stop_net_config();

	if (((global_net_mode == NET_MODE_BOOT_STA || global_net_mode == NET_MODE_CFG_STA ||
	      global_net_mode == NET_MODE_SW_STA) && infor.wifi_mode != STA) ||
	    global_net_mode == NET_MODE_INVALID || global_net_mode == NET_MODE_SW_NETCFG ||
	    global_net_mode == NET_MODE_CFG_START || global_net_mode == NET_MODE_CFG_CANCEL) {
		if (network_switch_sta_mode(5 * 60))
			global_net_mode = NET_MODE_SW_STA_ALLTIME;
	}

	module_mutex_unlock(&net_mode_lock);

	return 0;
}

static void net_module_next_module(struct mozart_module_struct *self)
{
	if (global_net_mode != NET_MODE_BOOT_STA)
		mozart_bt_avk_start(false);
}

static struct mozart_module_struct net_module = {
	.name = "net",
	.attach = module_attach_do_not_care,
	.priority = 1,
	.mops = {
		.on_start   = net_module_start,
		.on_run     = net_module_run,
		.on_suspend = net_module_suspend,
		.on_stop    = net_module_stop,
	},
	.kops = {
		.next_module = net_module_next_module,
	},
};

/*******************************************************************************
 * Function
 *******************************************************************************/
static bool switch_net_config(void)
{
	wifi_ctl_msg_t new_mode;

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));

	new_mode.force = true;
	new_mode.cmd = SW_NETCFG;
	new_mode.param.network_config.timeout = 120;
	strcpy(new_mode.name, global_app_name);
	strcpy(new_mode.param.network_config.wl_module, netcfg_method_str[BROADCOM]);
	strcpy(new_mode.param.network_config.product_model, "WONDERS_ENTERTAINMENT_ATALK_DS1825");
//	new_mode.param.network_config.method |= 0x01;
   //new_mode.param.network_config.method |= 0x01;
   //ATALK = 0x01 or AIRKISS_WE = 0x04 means can use airkiss and atalk at the same time,new_mode.param.network_config.method |= 0x05
//   new_mode.param.network_config.method |= 0x05;		//		atalk  (0x01) + airkiss (0x04)
#if (SUPPORT_NETWORK == NETWORK_ATALK_AIRKISS)
	new_mode.param.network_config.method |= 0x0C;		//		cooee  (0x08) + airkiss (0x04)
#elif(SUPPORT_NETWORK == NETWORK_COOEE_AIRKISS)
	new_mode.param.network_config.method |= 0x05;		//		atalk  (0x01) + airkiss (0x04)
#else
	new_mode.param.network_config.method |= 0x05;		//		atalk  (0x01) + airkiss (0x04)
#endif
	if (request_wifi_mode(new_mode)) {
		return true;
	}else{
		pr_err("Request SW_NETCFG Failed\n");
		return false;
	}
}

static bool stop_net_config(void)
{
	wifi_ctl_msg_t new_mode;

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));

	new_mode.force = true;
	new_mode.cmd = STOP_NETCFG;
	strcpy(new_mode.name, global_app_name);

	if (request_wifi_mode(new_mode)) {
		return true;
	} else {
		pr_err("Request STOP_NETCFG Failed\n");
		return false;
	}
}

static void __net_config_func(void)
{
	int ret;

	if (global_net_mode == NET_MODE_SW_NETCFG ||
	    global_net_mode == NET_MODE_CFG_CANCEL) {
		pr_debug("net_mode: %s\n", net_mode_str[global_net_mode]);
		return ;
	} else if (global_net_mode == NET_MODE_CFG_START) {
		if (stop_net_config())
			global_net_mode = NET_MODE_CFG_CANCEL;
		return ;
	} else {
		global_net_mode = NET_MODE_SW_NETCFG;
		module_mutex_unlock(&net_mode_lock);

		ret = net_module.start(&net_module, module_cmd_stop, false);
		if (ret) {
			pr_err("start fail\n");
			return ;
		}

		module_mutex_lock(&net_mode_lock);
		if (global_net_mode != NET_MODE_SW_NETCFG)
			return ;

		stopall(APP_DEPEND_NET_ALL);
#if SUPPORT_SMARTUI
		mozart_smartui_net_start();
#endif
		switch_net_config();
		module_mutex_unlock(&net_mode_lock);

		mozart_module_mutex_lock();
		if (__mozart_module_is_start(&net_module))
			mozart_prompt_tone_key_sync("atalk_wifi_config_7", true);
		mozart_module_mutex_unlock();

		module_mutex_lock(&net_mode_lock);
		return ;
	}
}

static void network_stop_wifi_mode(void)
{
	wifi_ctl_msg_t new_mode;

	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));
	new_mode.cmd = STOP_WIFI;
	strcpy(new_mode.name, global_app_name);
	if (!request_wifi_mode(new_mode))
		pr_err("Request STOP_WIFI Failed\n");
}

static bool network_switch_sta_mode(int timeout)
{
	wifi_ctl_msg_t new_mode;

	network_stop_wifi_mode();
	memset(&new_mode, 0, sizeof(wifi_ctl_msg_t));
	new_mode.cmd = SW_STA;
	new_mode.force = true;
	new_mode.param.switch_sta.sta_timeout = timeout;
	strcpy(new_mode.name, global_app_name);
	if (request_wifi_mode(new_mode))
		return true;
	else
		pr_err("Request SW_STA Failed\n");

	return false;
}

static bool network_is_str(char *a, const char *b)
{
	if (a == NULL || b == NULL)
		return false;
	else
		return !strncmp(a, b, strlen(b)) && (strlen(a) == strlen(b));
}

static bool network_event_is_type(event_info_t *event_ptr, event_notice_t type)
{
	return network_is_str(event_ptr->type, event_type_str[type]);
}

static bool network_content_is_configure_status(event_info_t *event_ptr, enum airkiss_operation status)
{
	return network_is_str(event_ptr->content, network_configure_status_str[status]);
}

static char *network_sta_error_str(char *reason)
{
	if (network_is_str(reason, sta_error_str[NONEXISTENCE_WPAFILE]) ||
	    network_is_str(reason, sta_error_str[NON_VALID_SSID])) {
		return "网络连接失败";
	} else if (network_is_str(reason, sta_error_str[WRONG_KEY])) {
		return "WIFI密码错误";
	} else if (network_is_str(reason, sta_error_str[SCAN_SSID_FAILED])) {
		return "找不到可用WIFI";
	} else if (network_is_str(reason, sta_error_str[CONNECT_TIMEOUT])) {
		return "网络连接超时";
	}

	return "";
}

static bool network_sta_error_handler(char *reason, bool display)
{
	#if SUPPORT_SMARTUI
	char *s;
	#endif
	if (reason == NULL)
		return true;

	if (network_is_str(reason, sta_error_str[WRONG_KEY])) {
		wrong_key = true;
		return false;
	} else if (wrong_key && network_is_str(reason, sta_error_str[SCAN_SSID_FAILED])) {
		wrong_key = false;
		#if SUPPORT_SMARTUI
		if (display) {
			s = network_sta_error_str((char *)sta_error_str[WRONG_KEY]);
			mozart_smartui_boot_display(s);
		}
		#endif
		return true;
	} else {
		wrong_key = false;
		#if SUPPORT_SMARTUI
		if (display) {
			s = network_sta_error_str(reason);
			mozart_smartui_boot_display(s);
		}
		#endif
		return true;
	}
}

static int __network_configure_error_handler(char *s)
{
	global_net_mode = NET_MODE_SW_STA;
#if SUPPORT_SMARTUI
	mozart_smartui_net_fail(s);
#endif
	module_mutex_unlock(&net_mode_lock);

	/* Can stop tone */
	mozart_prompt_tone_key_sync("atalk_wifi_config_fail_9", false);

	module_mutex_lock(&net_mode_lock);
	if (global_net_mode != NET_MODE_SW_STA)
		return -1;
#if SUPPORT_SMARTUI
	mozart_smartui_boot_build_display("尝试恢复网络连接");
#endif
	usleep(1000 * 1000);

	return 0;
}

static inline void network_configure_handler(event_info_t network_event, struct json_object *wifi_event)
{
	enum atalk_network_state network_state;
	pr_debug("-----------> network_configure_handler\n");
	if (network_content_is_configure_status(&network_event, AIRKISS_STARTING)) {
		/* net config start */
		module_mutex_lock(&net_mode_lock);
		if (global_net_mode == NET_MODE_SW_NETCFG) {
			global_net_mode = NET_MODE_CFG_START;
		} else {
			stop_net_config();
			pr_err("netcfg start, net_mode is %s ?\n", net_mode_str[global_net_mode]);
			module_mutex_unlock(&net_mode_lock);
			return ;
		}
		module_mutex_unlock(&net_mode_lock);

		mozart_module_mutex_lock();
		if (__mozart_net_is_start()) {

			#if (SUPPORT_VR == VR_ATALK)
				network_state = __mozart_atalk_network_state();
			#elif (SUPPORT_VR == VR_SPEECH)
				network_state = __mozart_aitalk_network_state();
			#endif

			__mozart_module_set_offline();
			__mozart_module_set_net();
			#if (SUPPORT_VR == VR_ATALK)
			__mozart_atalk_network_trigger(network_config, network_state, true);
			#elif (SUPPORT_VR == VR_SPEECH)
			__mozart_aitalk_network_trigger(network_config, network_state, true);
			#endif

			if (!__mozart_module_is_attach()){
				#if (SUPPORT_VR == VR_ATALK)
					__mozart_atalk_switch_mode(true);
				#elif (SUPPORT_VR == VR_SPEECH)
					__mozart_aitalk_switch_mode(true);
				#endif
			}
		} else {
			/* net_mode is on_stop */
			__mozart_module_set_offline();
			__mozart_module_set_net();
		}
		mozart_module_mutex_unlock();

	} else if (network_content_is_configure_status(&network_event, AIRKISS_SUCCESS)) {
		/* net config success */
		module_mutex_lock(&net_mode_lock);
		if (global_net_mode == NET_MODE_CFG_START) {
			global_net_mode = NET_MODE_CFG_STA;
		} else {
			pr_err("netcfg success, net_mode is %s ?\n", net_mode_str[global_net_mode]);
			module_mutex_unlock(&net_mode_lock);
			return ;
		}
		module_mutex_unlock(&net_mode_lock);
#if SUPPORT_SMARTUI
		mozart_smartui_net_receive_success();
#endif
	} else if ((network_content_is_configure_status(&network_event, AIRKISS_FAILED)) ||
		   network_content_is_configure_status(&network_event, AIRKISS_CANCEL)) {
		/* net config fail/cancel */
		module_mutex_lock(&net_mode_lock);
		if (global_net_mode == NET_MODE_CFG_START ||
		    global_net_mode == NET_MODE_CFG_CANCEL) {
			struct json_object *tmp = NULL;
			char *reason = NULL, *s = NULL;

			if (network_content_is_configure_status(&network_event, AIRKISS_FAILED)) {
				json_object_object_get_ex(wifi_event, "reason", &tmp);
				if (tmp)
					reason = (char *)json_object_get_string(tmp);

				if (reason && network_is_str(reason, netcfg_error_str[INVALID_PWD_LEN]))
					s = "密码长度无效";
			}

			if (__network_configure_error_handler(s)) {
				module_mutex_unlock(&net_mode_lock);
				return ;
			}

			network_switch_sta_mode(30);
		} else {
			pr_err("netcfg fail, net_mode is %s ?\n", net_mode_str[global_net_mode]);
			module_mutex_unlock(&net_mode_lock);
			return ;
		}
		module_mutex_unlock(&net_mode_lock);
	}
}

static inline void network_sta_failed_handler(char *reason)
{
	bool force = true;
	enum atalk_network_state network_state;

	if (reason == NULL)
		return ;

	module_mutex_lock(&net_mode_lock);
	if (global_net_mode == NET_MODE_BOOT_STA) {
		if (network_sta_error_handler(reason, true)) {
			module_mutex_unlock(&net_mode_lock);
			/* Can stop tone */
			mozart_prompt_tone_key_sync("atalk_offline_2", false);

			module_mutex_lock(&net_mode_lock);
			if (global_net_mode == NET_MODE_BOOT_STA)
				__net_config_func();
		}

		module_mutex_unlock(&net_mode_lock);
		return ;
	} else if (global_net_mode == NET_MODE_CFG_STA) {
		char *s = network_sta_error_str(reason);

		__network_configure_error_handler(s);

		module_mutex_unlock(&net_mode_lock);
		return ;
	} else if (global_net_mode == NET_MODE_SW_STA) {
		pr_debug("switch sta fail\n");
		if (network_sta_error_handler(reason, true)) {
			usleep(1000 * 1000);
			if (network_switch_sta_mode(5 * 60))
				global_net_mode = NET_MODE_SW_STA_ALLTIME;
		} else {
			module_mutex_unlock(&net_mode_lock);
			return ;
		}
	} else if (global_net_mode == NET_MODE_SW_STA_ALLTIME) {
		pr_debug("switch sta alltime\n");
		if ((network_is_str(reason, sta_error_str[NONEXISTENCE_WPAFILE]) ||
		     network_is_str(reason, sta_error_str[NON_VALID_SSID])))
			network_stop_wifi_mode();
		else if ((network_is_str(reason, sta_error_str[CONNECT_TIMEOUT]) ||
			  network_is_str(reason, sta_error_str[CONNECT_TIMEOUT_ALLTIME])))
			network_switch_sta_mode(5 * 60);

		module_mutex_unlock(&net_mode_lock);
		return ;
	} else if (global_net_mode == NET_MODE_STA) {
		pr_debug("wifi disconnect\n");
		force = false;
		if (network_switch_sta_mode(5 * 60))
			global_net_mode = NET_MODE_SW_STA_ALLTIME;
	} else {
		pr_err("sta fail, net_mode is %s ?\n", net_mode_str[global_net_mode]);
		module_mutex_unlock(&net_mode_lock);
		return ;
	}
	stopall(APP_DEPEND_NET_ALL);
	module_mutex_unlock(&net_mode_lock);

	mozart_module_mutex_lock();

	#if (SUPPORT_VR == VR_ATALK)
		network_state = __mozart_atalk_network_state();
	#elif (SUPPORT_VR == VR_SPEECH)
		network_state = __mozart_aitalk_network_state();
	#endif


	__mozart_module_set_offline();
	__mozart_module_clear_net();

	#if (SUPPORT_VR == VR_ATALK)
		__mozart_atalk_network_trigger(network_offline, network_state, force);
	#elif (SUPPORT_VR == VR_SPEECH)
		__mozart_aitalk_network_trigger(network_offline, network_state, force);
	#endif

	mozart_module_mutex_unlock();
}

static inline void network_wifi_mode_handler(event_info_t network_event)
{
	bool force = true;
	enum atalk_network_state network_state;

	if (network_is_str(network_event.content, wifi_mode_str[STA])) {
		module_mutex_lock(&net_mode_lock);
		wrong_key = false;
		if (global_net_mode == NET_MODE_STA) {
#if 0
			if (__mozart_module_is_attach()) {
				if (net_module.start(&net_module, module_cmd_stop, false)) {
					pr_err("connect fail!\n");
					module_mutex_unlock(&net_mode_lock);
					return ;
				}
			#if SUPPORT_SMARTUI
				mozart_smartui_boot_build_display("网络断开,已重连");
			#endif
				usleep(1000 * 1000);
			}
			stopall(APP_DEPEND_NET_ALL);
			startall(APP_DEPEND_NET_ALL);
			module_mutex_unlock(&net_mode_lock);

			mozart_module_mutex_lock();
			__mozart_atalk_network_trigger(network_config, network_online, false);
			__mozart_atalk_network_trigger(network_online, network_config, false);
			mozart_module_mutex_unlock();
#else
			module_mutex_unlock(&net_mode_lock);
#endif
			printf("\n\n-NET_MODE_STA--1111111111111-----------\n\n");
			return ;
		} else if (global_net_mode == NET_MODE_SW_STA_ALLTIME) {

			printf("\n\n-NET_MODE_SW_STA_ALLTIME--2222222222222-----------\n\n");
			global_net_mode = NET_MODE_STA;
			force = false;
		} else if (global_net_mode == NET_MODE_BOOT_STA) {

			printf("\n\n-NET_MODE_BOOT_STA | CFG_STA --3333333333333-----------\n\n");
			global_net_mode = NET_MODE_STA;
			#if (SUPPORT_VR == VR_ATALK)
				#if SUPPORT_SMARTUI
					mozart_smartui_boot_build_display("正在连接网络服务");
				#endif
			#elif (SUPPORT_VR == VR_SPEECH)
				#if SUPPORT_SMARTUI
					mozart_smartui_boot_build_display("网络连接成功");
				#endif
				mozart_prompt_tone_key_sync("atalk_wifi_config_success_8", false);
				__mozart_module_set_online();
				#if SUPPORT_SMARTUI
					mozart_smartui_net_success();
				#endif
				printf("\n\n- online AAA -----------\n\n");
			//	mozart_aitalk_cloudplayer_start(true);
			#endif
		} else if (global_net_mode == NET_MODE_CFG_STA) {
			printf("\n\n-NET_MODE_CFG_ST  --555555555555555 -----------\n\n");
			global_net_mode = NET_MODE_STA;
			#if (SUPPORT_VR == VR_ATALK)
				#if SUPPORT_SMARTUI
					mozart_smartui_net_success();
				#endif
				mozart_prompt_tone_key_sync("atalk_wifi_config_success_8", false);
			#elif (SUPPORT_VR == VR_SPEECH)
				#if SUPPORT_SMARTUI
					mozart_smartui_net_success();
				#endif
				mozart_prompt_tone_key_sync("atalk_wifi_config_success_8", false);
				printf("\n\n- online BBBB -----------\n\n");
				mozart_aitalk_cloudplayer_start(true);
			#endif
		} else if (global_net_mode == NET_MODE_SW_STA) {

			printf("\n\n---------net--4444444444444-----------\n\n");
			global_net_mode = NET_MODE_STA;
			#if SUPPORT_SMARTUI
				mozart_smartui_boot_build_display("网络连接已恢复");
			#endif
		}/* else if (global_net_mode == NET_MODE_CFG_STA) {

			printf("\n\n---------net--55555555555555-----------\n\n");
			global_net_mode = NET_MODE_STA;
		#if SUPPORT_SMARTUI
			mozart_smartui_net_success();
		#endif
			mozart_prompt_tone_key_sync("atalk_wifi_config_success_8", false);
		} //*/else {

			printf("\n\n---------net--66666666666666-----------\n\n");
			pr_err("sta mode, net_mode is %s ?\n", net_mode_str[global_net_mode]);
			module_mutex_unlock(&net_mode_lock);
			return ;
		}
		startall(APP_DEPEND_NET_ALL);
		module_mutex_unlock(&net_mode_lock);

		mozart_module_mutex_lock();

		#if (SUPPORT_VR == VR_ATALK)
			network_state = __mozart_atalk_network_state();
		#elif (SUPPORT_VR == VR_SPEECH)
			network_state = __mozart_aitalk_network_state();
		#endif


		__mozart_module_set_online();
		__mozart_module_clear_net();

		#if (SUPPORT_VR == VR_ATALK)
			__mozart_atalk_network_trigger(network_online, network_state, force);
		#elif (SUPPORT_VR == VR_SPEECH)
			__mozart_aitalk_network_trigger(network_online, network_state, force);
		#endif

		mozart_module_mutex_unlock();
	} else {
		module_mutex_lock(&net_mode_lock);
		if (global_net_mode == NET_MODE_STA ||
		    network_is_str(network_event.content, wifi_mode_str[STA_WAIT])) {
			pr_err("WIFI_MODE: %s\n", network_event.content);
			if (network_switch_sta_mode(5 * 60))
				global_net_mode = NET_MODE_SW_STA_ALLTIME;

			stopall(APP_DEPEND_NET_ALL);
			module_mutex_unlock(&net_mode_lock);

			mozart_module_mutex_lock();

		#if (SUPPORT_VR == VR_ATALK)
			network_state = __mozart_atalk_network_state();
			__mozart_module_set_offline();
			__mozart_module_clear_net();
			__mozart_atalk_network_trigger(network_offline, network_state, false);
		#elif (SUPPORT_VR == VR_SPEECH)
			network_state = __mozart_aitalk_network_state();
			__mozart_module_set_offline();
			__mozart_module_clear_net();
			__mozart_aitalk_network_trigger(network_offline, network_state, false);
		#endif


			mozart_module_mutex_unlock();
		} else {
			module_mutex_unlock(&net_mode_lock);
		}
	}
}

static int network_callback(const char *p)
{
	event_info_t network_event;
	struct json_object *wifi_event = NULL, *tmp = NULL;

	if (p == NULL)
		return -1;
	wifi_event = json_tokener_parse(p);
	pr_debug("network event: %s, net_mode is %s\n\n", p, net_mode_str[global_net_mode]);

	memset(&network_event, 0, sizeof(event_info_t));
	json_object_object_get_ex(wifi_event, "name", &tmp);
	if (tmp)
		strncpy(network_event.name, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
	json_object_object_get_ex(wifi_event, "type", &tmp);
	if (tmp)
		strncpy(network_event.type, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));
	json_object_object_get_ex(wifi_event, "content", &tmp);
	if (tmp)
		strncpy(network_event.content, json_object_get_string(tmp), strlen(json_object_get_string(tmp)));

	if (network_event_is_type(&network_event, NETWORK_CONFIGURE)) {
		network_configure_handler(network_event, wifi_event);
	} else if (network_event_is_type(&network_event, STA_STATUS)) {
		if (network_is_str(network_event.content, "STA_CONNECT_FAILED")) {
			char *reason = NULL;

			json_object_object_get_ex(wifi_event, "reason", &tmp);
			if (tmp)
				reason = (char *)json_object_get_string(tmp);

			network_sta_failed_handler(reason);
		}
	} else if (network_event_is_type(&network_event, WIFI_MODE)) {
		network_wifi_mode_handler(network_event);
	}

	json_object_put(wifi_event);

	return 0;
}

static void *net_config_func(void *args)
{
	pthread_detach(pthread_self());
	module_mutex_lock(&net_mode_lock);
	__net_config_func();
	module_mutex_unlock(&net_mode_lock);

	return NULL;
}

int create_wifi_config_pthread(void)
{
	/* enter wifi config mode. */
	if (pthread_create(&net_config_pthread, NULL, net_config_func, NULL) == -1) {
		pr_err("Create wifi config pthread failed: %s.\n", strerror(errno));
		return -1;
	}
//	pthread_detach(net_config_pthread);

	return 0;
}

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_net_is_start(void)
{
	return __mozart_module_is_start(&net_module);
}

int mozart_net_startup(void)
{
	int try_cnt = 0;
	struct wifi_client_register wifi_info;

	if (mozart_module_register(&net_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	/* wait for 5s if network is invalid now. */
	try_cnt = 0;
	while (try_cnt++ < 10) {
		if (!access("/var/run/wifi-server/register_socket", F_OK) &&
		    !access("/var/run/wifi-server/server_socket", F_OK))
			break;
		usleep(500 * 1000);
	}
	if (try_cnt >= 10) {
		printf("[Warning] %s: Can't connect to networkmanager\n", __func__);
		return -1;
	}

	/* register network manager */
	wifi_info.pid = getpid();
	wifi_info.reset = 1;
	wifi_info.priority = 3;
	strcpy(wifi_info.name, global_app_name);
	register_to_networkmanager(wifi_info, network_callback);

	if (net_module.start(&net_module, module_cmd_stop, false)) {
		pr_err("connect fail!\n");
		return -1;
	}

	if (network_switch_sta_mode(60))
		global_net_mode = NET_MODE_BOOT_STA;
	else
		pr_err("Request SW_STA Failed\n");
#if SUPPORT_SMARTUI
	mozart_smartui_boot_build_display("正在连接网络");
#endif
	return 0;
}

void mozart_net_shutdown(void)
{
	if (net_module.stop)
		net_module.stop(&net_module, module_cmd_stop, false);
	mozart_module_unregister(&net_module);

	unregister_from_networkmanager();
}
