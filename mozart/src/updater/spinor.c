#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "nvrw_interface.h"
#include "updater_interface.h"
#include "tips_interface.h"
#include "flash.h"
#include "debug.h"
#include "common.h"

static int update_usrdata(void)
{
	return 0;
}

int spinor_process_update(struct nv_info *info)
{
	int cnt = 0;
	int ret = 0;
	int nv_lock = -1;

	if (info->update_method.method == UPDATE_ONCE) {
		pr_warn("spinor CANOT support UPDATE_ONCE update method right now!!\n");
		return -1;
	}

	if (updater_connect_wifi()) {
		pr_err("connect to wifi error, reboot.\n");
		system("reboot");
	}

	mozart_play_key_sync("updating");

	cnt = 12;
	while (cnt-- > 0) {
		ret = mozart_updater_download(info);
		if (ret) {
			pr_warn("download error, #%d try left.\n", cnt);
			sleep(5);
			continue;
		} else {
			system("sync");
			break;
		}
	}
	if (cnt < 0) {
		mozart_play_key_sync("update_failed");
		pr_warn("update fail.\n");
		system("reboot");
	}

	ret = mozart_updater_update(info);
	if (ret) {
		pr_err("update error.\n");
		system("reboot");
		return -1;
	}

	info->update_process += 1;

	if (info->update_process == PROCESS_DONE) {
		info->update_flag = 0;
		strcpy(info->current_version, info->update_version);
		update_usrdata();
		mozart_play_key_sync("update_success");
	}

	nv_lock = mozart_nv_lock();
	mozart_nv_set_nvinfo(info);
	mozart_nv_unlock(nv_lock);
	system("sync");

	system("reboot");

	return ret;
}

int spinor_prepare_new_version(struct nv_info *info)
{
	return 0;
}
