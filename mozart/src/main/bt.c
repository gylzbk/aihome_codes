#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "utils_interface.h"
#include "bluetooth_interface.h"
#include "sharememory_interface.h"
#include "mozart_config.h"

#if (SUPPORT_WEBRTC == 1)
#include "webrtc_aec.h"
bt_aec_callback bt_ac;
#endif

void bt_info_init(bt_init_info *bt_info, char *bt_name)
{
	bt_info->bt_name = bt_name;
	bt_info->discoverable = 1;
	bt_info->connectable = 1;
	memset(bt_info->out_bd_addr, 0, sizeof(bt_info->out_bd_addr));
}

void *thr_fn(void *args)
{
	int i = 0;
	bt_init_info bt_info;
	char mac[] = "00:11:22:33:44:55";
	char bt_name[64] = {};
	char bt_avk_name[25] = "/var/run/bt-avk-fifo";
	char bt_socket_name[30] = "/var/run/bt-daemon-socket";

	for(i = 0; i < 100; i++) {
		if(!access(bt_socket_name, 0) && !access(bt_avk_name, 0)) {
			break;
		} else {
			usleep(50000);
		}
	}

	if(access(bt_socket_name, 0) || access(bt_avk_name, 0)) {
		printf("%s or %s not exists, please check !!\n",
				bt_avk_name, bt_socket_name);
		goto err_exit;
	}

	memset(mac, 0, sizeof(mac));
	memset(bt_name, 0, sizeof(bt_name));
	get_mac_addr("wlan0", mac, "");

	strcat(bt_name, "SmartAudio-");
	strcat(bt_name, mac+4);
	bt_info_init(&bt_info, bt_name);
#if (SUPPORT_BT == BT_RTK)
	system("bt_enable &");
#elif (SUPPORT_BT == BT_BCM)
	printf("Bluetooth name is: %s\n", bt_name);
	if (mozart_bluetooth_init(&bt_info)) {
		printf("bluetooth init failed.\n");
		goto err_exit;
	}

	if (mozart_bluetooth_hs_start_service()) {
		printf("hs service start failed.\n");
		goto err_exit;
	}

	if (mozart_bluetooth_avk_start_service()) {
		printf("avk service start failed.\n");
		goto err_exit;
	}
#elif (SUPPORT_BT == BT_NULL)
	printf("Bt funcs are closed.\n");
	goto err_exit;
#else
#error "Not supported bt module found."
#endif

err_exit:

	return NULL;
}

int start_bt(void)
{
	int err;
	pthread_t p_tid;

	system("/usr/fs/etc/init.d/S04bsa.sh start");

#if (SUPPORT_WEBRTC == 1)
	bt_ac.aec_init = ingenic_apm_init;
	bt_ac.aec_destroy = ingenic_apm_destroy;
	bt_ac.aec_enable = webrtc_aec_enable;
	bt_ac.aec_get_buffer_length = webrtc_aec_get_buffer_length;
	bt_ac.aec_calculate = webrtc_aec_calculate;

	mozart_aec_callback(&bt_ac);
#else
	mozart_aec_callback(NULL);
#endif

	err = pthread_create(&p_tid, NULL, thr_fn, NULL);
	if (err != 0)
		printf("can't create thread: %s\n", strerror(err));

	pthread_detach(p_tid);

	return 0;
}

int stop_bt(void)
{
#if 0
	system("bt_disable");
#else
	//TODO: DO nothing now.
	mozart_bluetooth_disconnect();
	mozart_bluetooth_hs_stop_service();
	mozart_bluetooth_avk_stop_service();
	mozart_bluetooth_uninit();
	system("/usr/fs/etc/init.d/S04bsa.sh stop");
#endif

	return 0;
}

static int btMode_set = 0;
static pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;

void bt_mode_set(void)
{
	pthread_mutex_lock(&mode_mutex);
	btMode_set = 1;
	pthread_mutex_unlock(&mode_mutex);
}

void bt_mode_clear(voyyid)
{
	pthread_mutex_lock(&mode_mutex);
	btMode_set = 0;
	pthread_mutex_unlock(&mode_mutex);
}

int is_bt_mode(void)
{
	return btMode_set;
}
