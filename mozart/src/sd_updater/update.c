#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "linklist_interface.h"
#include "utils_interface.h"
#include "nv_rw.h"

#ifdef SUPPORT_SMARTUI
#include "smartui_interface.h"
#endif

#ifndef UPDATE_RELEASE
#define UPDATE_DEBUG
#endif

#ifdef UPDATE_DEBUG
#define pr_debug(name, fmt, args...)	printf("%s.%s [Debug] "fmt, name, __func__, ##args)
#else  /* UPDATE_DEBUG */
#define pr_debug(name fmt, args...)
#endif /* UPDATE_DEBUG */

#define pr_info(name, fmt, args...)	printf("%s.%s [Info] "fmt, name, __func__, ##args)
#define pr_err(name, fmt, args...)	fprintf(stderr, "%s.%s [Error] "fmt, name, __func__, ##args)

enum update_item {
	OPT_BOOT = 0,
	OPT_USERDATA,
	OPT_KERNEL,
	OPT_ROOTFS,
	OPT_APPFS,
	OPT_ITEM_COUNT,
};

typedef struct {
	char		*name;

	char		*src_path;
	char		*dev_path;
	char		*block_dev;

	struct stat	st;
	size_t		src_size;
	int		erase_boundary; /* 0, whole block. */
} updateInfo_t;

static updateInfo_t info_array[] = {
	[OPT_BOOT] = {
		.name		= "u-boot",
		.src_path	= "/mnt/sdcard/update/u-boot-with-spl.bin",
		.dev_path	= "/dev/mtd0",
		.block_dev	= "/dev/mtdblock0",
		.erase_boundary	= 256 * 1024,
	},

	[OPT_USERDATA] = {
		.name		= "usrdata",
		.src_path	= "/mnt/sdcard/update/usrdata.jffs2",
		.dev_path	= "/dev/mtd1",
		.block_dev	= "/dev/mtdblock1",
		.erase_boundary = 0,
	},

	[OPT_KERNEL] = {
		.name		= "kernel",
		.src_path	= "/mnt/sdcard/update/zImage",
		.dev_path	= "/dev/mtd2",
		.block_dev	= "/dev/mtdblock2",
		.erase_boundary = 0,
	},

	[OPT_ROOTFS] = {
		.name		= "rootfs",
		.src_path	= "/mnt/sdcard/update/updater.cramfs",
		.dev_path	= "/dev/mtd3",
		.block_dev	= "/dev/mtdblock3",
		.erase_boundary	= 0,
	},

	[OPT_APPFS] = {
		.name		= "appfs",
		.src_path	= "/mnt/sdcard/update/appfs.cramfs",
		.dev_path	= "/dev/mtd4",
		.block_dev	= "/dev/mtdblock4",
		.erase_boundary	= 0,
	},
};

static char *appName = NULL;
static char command[256];

#ifdef SUPPORT_SMARTUI
#define UPDATE_SMARTUI_PATH	"/usr/fs/usr/share/ui/"
static struct view_struct global_background_view = {
	.left = 0,
	.top = 0,
	.right = 240,
	.bottom = 320,
	.rgb = 0,
	.layer = bottom_layer,
	.align = center_align,
};

struct view_struct view = {
	.left = 20,
	.top = 190,
	.right = 220,
	.bottom = 215,
	.layer = top_layer,
	.align = left_align,
};

struct imageview_struct *update_background_imageview;
struct textview_struct *update_prompt_textview;

static void mozart_update_smartui_init(void)
{
	smartui_startup();
	system("echo 120 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");

	update_background_imageview = smartui_imageview(&global_background_view);
	if (update_background_imageview == NULL)
		pr_err(appName, "build update_background_imageview failed!\n");

	view.bottom_view = &update_background_imageview->v;

	update_prompt_textview = smartui_textview(&view);
	if (update_prompt_textview == NULL)
		pr_err(appName, "build update_prompt_textview failed!\n");

	smartui_imageview_display(update_background_imageview,  UPDATE_SMARTUI_PATH"1_240_320.bmp");
}

static void mozart_update_smartui_shutdown(void)
{
	system("echo 0 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");
	smartui_shutdown();
}

static void mozart_update_smartui_update_show(int ratio_p, int ratio_s)
{
	char ratio_str[48] = {0};

	if (ratio_p && ratio_s)
		sprintf(ratio_str, "固件更新中...%d/%d", ratio_p, ratio_s);
	else
		sprintf(ratio_str, "固件更新中");

	smartui_textview_set_align(update_prompt_textview, center_align);
	smartui_textview_display(update_prompt_textview, ratio_str);
	smartui_sync();
}
#endif /* SUPPORT_SMARTUI */

#define ERASE_BLOCK	(32 * 1024)
#define DELIMIT_LINE	"======================================="

int main(int argc, char *argv[])
{
	struct nv_info nv;
	int i;
	int err;

	appName = basename(argv[0]);

	err = nvrw_get_update_info(&nv);
	if (err < 0) {
		pr_err(appName, "get nv state failed\n");
		return -1;
	}

	/* Check update flag */
	if (!nv.update_status)
		return 0;

#ifdef SUPPORT_SMARTUI
	int fin = 0;
	int file_count = 0;

	/* Init smart lcd */
	mozart_update_smartui_init();
	/* Display updating prompt */
	mozart_update_smartui_update_show(0, 0);
#endif

	/* Umount Usrdata partition */
	umount("/usr/data");
	/* Umount App partition */
	umount("/usr/fs");

	/* Simple update */
	/* Check files first */
	for (i = 0; i < OPT_ITEM_COUNT; i++) {
		err = stat(info_array[i].src_path, &info_array[i].st);
		if (err < 0) {
			if (errno == ENOENT) { /* File not found */
				pr_info(appName, "%s %s, not update\n", info_array[i].src_path, strerror(errno));
				info_array[i].src_size = 0;
				continue;
			} else {
				pr_err(appName, "get %s stat: %s\n", info_array[i].name, strerror(errno));
				goto err_stat;
			}
		}

		info_array[i].src_size = info_array[i].st.st_size;
#ifdef SUPPORT_SMARTUI
		file_count++;
#endif
	}

	for (i = 0; i < OPT_ITEM_COUNT; i++) {
		if (!info_array[i].src_size) {
			/* Need not update */
			continue;
		}

#ifdef SUPPORT_SMARTUI
		fin++;
		mozart_update_smartui_update_show(fin, file_count);
#endif

		printf("%s\n", DELIMIT_LINE);

		/* Erase update partition */
		if (!strcmp(info_array[i].name, "u-boot"))
			pr_info(appName, "[DANGER OPERATION] Erase %s partition\n", info_array[i].name);

		pr_info(appName, "Erasing %s\n", info_array[i].dev_path);

		sprintf(command, "flash_erase -q %s 0 %d",
			info_array[i].dev_path,
			info_array[i].erase_boundary ? info_array[i].erase_boundary / ERASE_BLOCK : 0);
		err = mozart_system(command);
		if (err == -1) {
			pr_err(appName, "Run system failed\n");
			goto err_par_erase;
		} else if (WEXITSTATUS(err)) {
			pr_err(appName, "Erase %s: %s\n", info_array[i].dev_path, strerror(errno));
			goto err_par_erase;
		}

		pr_info(appName, "Write %s\n", info_array[i].src_path);
		sprintf(command, "dd if=%s of=%s bs=512", info_array[i].src_path, info_array[i].dev_path);
		err = mozart_system(command);
		if (err == -1) {
			pr_err(appName, "Run system failed\n");
			goto err_update_write;
		} else if (WEXITSTATUS(err)) {
			pr_err(appName, "Write %s: %s\n", info_array[i].src_path, strerror(errno));
			goto err_update_write;
		}

		printf("%s\n", DELIMIT_LINE);
	}

	/* Recovery wifi and bsa config */
	if (info_array[OPT_USERDATA].src_size) {
		printf("%s\n", DELIMIT_LINE);
		pr_info(appName, "Mount %s\n", info_array[OPT_USERDATA].name);

		err = mount(info_array[OPT_USERDATA].block_dev,
			    "/usr/data",
			    "jffs2",
			    0, NULL);
		if (err < 0) {
			pr_err(appName, "mount %s: %s\n", info_array[OPT_USERDATA].block_dev, strerror(errno));
			goto err_usr_mount;
		}

		pr_info(appName, "Copy wifi and bsa config\n");
		mozart_system("cp -r /mnt/sdcard/data/bsa /usr/data");
		mozart_system("cp -r /mnt/sdcard/data/wpa_supplicant.conf /usr/data");
		mozart_system("find /usr/data | xargs chmod -x");

		umount("/usr/data");
		printf("%s\n", DELIMIT_LINE);
	}

	/* Update success */
	nvrw_clear_update_flag();

	/* Clear update */
	if (access("/mnt/sdcard/update.ini", F_OK) == -1) {
		pr_info(appName, "Claer update firmware\n");
		mozart_system("rm -rf /mnt/sdcard/update");
		mozart_system("rm -rf /mnt/sdcard/zImage-ramfs");
		mozart_system("rm -rf /mnt/sdcard/data");
	}

#ifdef SUPPORT_SMARTUI
	/* Close smart lcd */
	mozart_update_smartui_shutdown();
#endif

	/* Reboot with sync */
	mozart_system("reboot -d 1");

	return 0;

err_usr_mount:
err_update_write:
err_par_erase:
err_stat:
	return -1;
}
