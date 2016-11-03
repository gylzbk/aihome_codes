#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "wifi_interface.h"

int updater_connect_wifi(void)
{
	int cnt = 0;
	struct wifi_client_register wifi_info;
	wifi_ctl_msg_t new_mode;
	wifi_info_t info;

	// register network manager
	memset(&wifi_info, 0, sizeof(wifi_info));
	wifi_info.pid = getpid();
	wifi_info.reset = 1;
	wifi_info.priority = 3;
	strcpy(wifi_info.name, "updater");
	if (register_to_networkmanager(wifi_info, NULL) != 0) {
		printf("ERROR: [updater] register to Network Server Failed!!!!\n");
		return -1;
	}

	memset(&new_mode, 0, sizeof(new_mode));
	new_mode.cmd = SW_STA;
	new_mode.force = true;
	strcpy(new_mode.name, "updater");
	if (request_wifi_mode(new_mode) != true) {
		printf("ERROR: [updater] Request sta mode Failed, Please Register First!!!!\n");
		return -1;
	}

	/* waiting for network ready. */
	cnt = 1200;
	while (cnt-- > 0) {
		info = get_wifi_mode();
		if (info.wifi_mode == STA || info.wifi_mode == STANET)
			break;
		usleep(500 * 1000);
		printf("updater: waiting network ready!!\n");
	}
	if (cnt < 0)
		return -1;

	return 0;
}
