#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "nvrw_interface.h"
#include "updater_interface.h"
#include "tips_interface.h"
#include "debug.h"
#include "flash.h"

#include "spinor.h"
#include "spinand.h"
#include "emmc.h"

extern void stop_sound(void);

static void usage(const char *app_name)
{
	printf("%s - a ota tool.\n"
		   " -h - help (show this usage text)\n"
		   " -p - process new version(check new version and start update)\n"
		   " -u - update if in updating.\n"
		   " -c - check if in updating\n", app_name);

	return;
}

int process_update(struct nv_info *info)
{
	if (info->update_flag != 1)
		return -1;

	if (!strcmp((char *)info->update_method.storage, "spinor")) {
		return spinor_process_update(info);
	} else if (!strcmp((char *)info->update_method.storage, "spinand")) {
		return spinand_process_update(info);
	} else if (!strcmp((char *)info->update_method.storage, "emmc")) {
		return mmc_process_update(info);
	} else {
		pr_warn("storage: %s Not support now.\n", info->update_method.storage);
		return -1;
	}

	return 0;
}

static int prepare_new_version(void)
{
	int nv_lock = -1;
	struct nv_info *info = NULL;

	/* check new version */
	info = mozart_updater_chkver();
	if (!info) {
		pr_info("No new version.\n");
		free(info);
	} else {
		info->update_process = PROCESS_1;

		if (!strcmp((char *)info->update_method.storage, "spinor")) {
			if (spinor_prepare_new_version(info) != 0)
				goto prepare_err;
		} else if (!strcmp((char *)info->update_method.storage, "spinand")) {
			if (spinand_prepare_new_version(info) != 0)
				goto prepare_err;
		} else if (!strcmp((char *)info->update_method.storage, "emmc")) {
			if (mmc_prepare_new_version(info) != 0)
				goto prepare_err;
		} else {
			pr_warn("storage: %s Not support now.\n", info->update_method.storage);
			goto prepare_err;
		}

		/* set update flag */
		info->update_flag = FLAG_UPDATE;
		nv_lock = mozart_nv_lock();
		mozart_nv_set_nvinfo(info);
		mozart_nv_unlock(nv_lock);

		free(info);

		stop_sound();
		mozart_play_key_sync("update_found");

		system("sync");
		system("reboot");
	}

	return 0;

prepare_err:
	free(info);
	return -1;
}

static int check_updating(struct nv_info *info)
{
	return info->update_flag ? 0 : -1;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int nv_lock = -1;
	struct nv_info *info = NULL;
	int check_updating_only = 0;
	int process_newver = 0;
	int updating_only = 0;

	/* Get command line parameters */
	int c;
	while (1) {
		c = getopt(argc, argv, "cpuh");
		if (c < 0)
			break;
		switch (c) {
		case 'c':
			check_updating_only = 1;
			break;
		case 'p':
			process_newver = 1;
			break;
		case 'u':
			updating_only = 1;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return -1;
		}
	}

	/* ONLY accept 0 or 1 param at the same time. */
	if (check_updating_only) {
		if (process_newver || updating_only) {
			ret = -1;
			goto too_many_args;
		}
	} else if (process_newver) {
		if (check_updating_only || updating_only) {
			ret = -1;
			goto too_many_args;
		}
	} else if (updating_only) {
		if (check_updating_only || process_newver) {
			ret = -1;
			goto too_many_args;
		}
	} else {
		/* default we check new version, if has we start update. */
		process_newver = 1;
	}

	nv_lock = mozart_nv_lock();
	info = mozart_nv_get_nvinfo();
	mozart_nv_unlock(nv_lock);

	if (info == NULL) {
		fprintf(stderr, "Can't get nvinfo\n");
		return -1;
	}

	if (check_updating_only) {
		ret = check_updating(info);
		free(info);
		return ret;
	} else if (updating_only) {
		ret = process_update(info);
		free(info);
		return ret;
	} else if (process_newver) {
		free(info);
		return prepare_new_version();
	}

	return 0;

too_many_args:
	printf("Too many args, Please use -c OR -p OR -u.\n");

	return ret;
}
