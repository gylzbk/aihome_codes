#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "volume_interface.h"
#include "linklist_interface.h"
#include "power_interface.h"
#include "smartui_interface.h"

#include "mozart_module.h"

//#define SMARTUI_PATH "/usr/fs/usr/share/ui/"
#define SMARTUI_PATH "/mnt/sdcard/ui/"
#define HZK16 16

#ifndef MOZART_RELEASE
#define MOZART_SMARTUI_DEBUG
#endif

#ifdef MOZART_SMARTUI_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[SMARTUI] %s: "fmt, __func__, ##args)
#else  /* MOZART_SMARTUI_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_SMARTUI_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[SMARTUI] [Error] %s "fmt, __func__, ##args)

static char owner[64];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static struct view_struct global_background_view = {
	.left = 0,
	.top = 0,
	.right = 240,
	.bottom = 320,
	.rgb = 0,
	.layer = bottom_layer,
	.align = center_align,
};
static struct textview_struct *global_info_textview;
static struct imageview_struct *global_wifi_imageview;
static struct imageview_struct *global_update_imageview;
static struct imageview_struct *global_battery_imageview;
static struct imageview_struct *global_background_imageview;
static struct imageview_struct *bt_hs_background_imageview;
static struct imageview_struct *asr_background_imageview;
static struct imageview_struct *update_background_imageview;

static void mozart_smartui_sync(void)
{
	smartui_sync();
	usleep(80 * 1000);
}

static void mozart_smartui_hide_all(void)
{
	if (smartui_imageview_hide(global_background_imageview))
		pr_err("hide global_background_imageview fail!\n");
#if 0
	if (smartui_imageview_hide(bt_hs_background_imageview))
		pr_err("hide bt_hs_background_imageview fail!\n");
	if (smartui_imageview_hide(asr_background_imageview))
		pr_err("hide asr_background_imageview fail!\n");
	if (smartui_imageview_hide(update_background_imageview))
		pr_err("hide update_background_imageview fail!\n");
#endif
}

static struct textview_struct *boot_prompt_textview;
bool mozart_smartui_is_boot_view(void)
{
	return !strcmp(owner, "boot");
}

static int mozart_smartui_build_boot_view(void)
{
	struct view_struct view = {
		.left = 20,
		.top = 190,
		.right = 220,
		.bottom = 320,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "boot")) {
		smartui_clear_top(&global_background_imageview->v);

		boot_prompt_textview = smartui_textview(&view);
		if (boot_prompt_textview == NULL)
			pr_err("build boot_prompt_textview fail!\n");

		strncpy(owner, "boot", 64);
		return 1;
	}

	return 0;
}

static struct imageview_struct *net_result_imageview;
static struct textview_struct *net_prompt_textview;
bool mozart_smartui_is_net_view(void)
{
	return !strcmp(owner, "net");
}

static int mozart_smartui_build_net_view(void)
{
	struct view_struct view = {
		.left = 32,
		.top = 210,
		.right = 220,
		.bottom = 320,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "net")) {
		smartui_clear_top(&global_background_imageview->v);

		net_prompt_textview = smartui_textview(&view);
		if (net_prompt_textview == NULL)
			pr_err("build net_prompt_textview fail!\n");

		view.left = 70;
		view.top = 102;
		view.right = 170;
		view.bottom = 208;
		net_result_imageview = smartui_imageview(&view);
		if (net_result_imageview == NULL)
			pr_err("build net_result_imageview fail!\n");

		strncpy(owner, "net", 64);
		return 1;
	}

	return 0;
}

static struct textview_struct *atalk_vendor_textview;
static struct textview_struct *atalk_title_textview;
static struct textview_struct *atalk_artist_textview;
static struct imageview_struct *atalk_play_imageview;
static struct imageview_struct *atalk_vol_imageview;
bool mozart_smartui_is_atalk_view(void)
{
	return !strcmp(owner, "atalk");
}

static int mozart_smartui_build_atalk_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 46,
		.right = 240,
		.bottom = 78,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "atalk")) {
		smartui_clear_top(&global_background_imageview->v);

		atalk_vendor_textview = smartui_textview(&view);
		if (atalk_vendor_textview == NULL)
			pr_err("build atalk_vendor_textview fail!\n");

		view.left = 8;
		view.top = 185;
		view.right = 232;
		view.bottom = 215;
		view.layer = top_layer;
		view.align = center_align;
		atalk_title_textview = smartui_textview(&view);
		if (atalk_title_textview == NULL)
			pr_err("build atalk_title_textview fail!\n");

		view.left = 0;
		view.top = 225;
		view.right = 240;
		view.bottom = 241;
		view.layer = top_layer;
		view.align = center_align;
		atalk_artist_textview = smartui_textview(&view);
		if (atalk_artist_textview == NULL)
			pr_err("build atalk_artist_textview fail!\n");
		smartui_textview_font_set(atalk_artist_textview, HZK16);

		view.left = 104;
		view.top = 139;
		view.right = 136;
		view.bottom = 159;
		view.layer = top_layer;
		view.align = left_align;
		atalk_play_imageview = smartui_imageview(&view);
		if (atalk_play_imageview == NULL)
			pr_err("build atalk_play_imageview fail!\n");

		view.left = 0;
		view.top = 266;
		view.right = 240;
		view.bottom = 306;
		view.layer = top_layer;
		view.align = left_align;
		atalk_vol_imageview = smartui_imageview(&view);
		if (atalk_vol_imageview == NULL)
			pr_err("build atalk_vol_imageview fail!\n");

		strncpy(owner, "atalk", 64);
		return 1;
	}

	return 0;
}

static struct textview_struct *asr_prompt_textview;
static struct imageview_struct *asr_icon_imageview;
static int mozart_smartui_build_asr_view(void)
{
	struct view_struct view = {
		.left = 30,
		.top = 200,
		.right = 217,
		.bottom = 300,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &asr_background_imageview->v,
	};

	mozart_smartui_hide_all();
	smartui_imageview_appear(asr_background_imageview);

	if (asr_prompt_textview == NULL) {
		asr_prompt_textview = smartui_textview(&view);
		if (asr_prompt_textview == NULL)
			pr_err("build asr_prompt_textview fail!\n");
	}

	if (asr_icon_imageview == NULL) {
		view.left = 70;
		view.top = 92;
		view.right = 170;
		view.bottom = 192;
		view.layer = top_layer;
		view.align = center_align;
		asr_icon_imageview = smartui_imageview(&view);
		if (asr_icon_imageview == NULL)
			pr_err("build asr_icon_imageview fail!\n");
	}

	return 1;
}

static void mozart_smartui_build_global_update_view(void)
{
	struct view_struct view = {
		.left = 119,
		.top = 0,
		.right = 119 + 28,
		.bottom = 28,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (global_update_imageview == NULL) {
		global_update_imageview = smartui_imageview(&view);
		if (global_update_imageview == NULL)
			pr_err("build global_update_imageview fail!\n");
		smartui_imageview_set_global(global_update_imageview);
	}
}

static struct textview_struct *update_prompt0_textview;
static struct textview_struct *update_prompt1_textview;
static int mozart_smartui_build_update_view(void)
{
	struct view_struct view = {
		.left = 20,
		.top = 190,
		.right = 220,
		.bottom = 215,
		.layer = top_layer,
		.align = left_align,
		.bottom_view = &update_background_imageview->v,
	};

	mozart_smartui_hide_all();
	smartui_imageview_appear(update_background_imageview);

	if (update_prompt0_textview == NULL) {
		update_prompt0_textview = smartui_textview(&view);
		if (update_prompt0_textview == NULL)
			pr_err("build update_prompt0_textview fail!\n");
	}

	if (update_prompt1_textview == NULL) {
		view.top = 215;
		view.bottom = 240;
		update_prompt1_textview = smartui_textview(&view);
		if (update_prompt1_textview == NULL)
			pr_err("build update_prompt1_textview fail!\n");
	}

	return 1;
}

static struct textview_struct *bt_hs_prompt_textview;
static struct imageview_struct *bt_hs_imageview;
static int mozart_smartui_build_bt_hs_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 190,
		.right = 240,
		.bottom = 220,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &bt_hs_background_imageview->v,
	};

	if (bt_hs_prompt_textview == NULL) {
		bt_hs_prompt_textview = smartui_textview(&view);
		if (bt_hs_prompt_textview == NULL)
			pr_err("build bt_hs_prompt_textview fail!\n");
	}

	if (bt_hs_imageview == NULL) {
		view.left = 70;
		view.top = 80;
		view.right = 170;
		view.bottom = 180;
		view.layer = top_layer;
		view.align = left_align;
		bt_hs_imageview = smartui_imageview(&view);
		if (bt_hs_imageview == NULL)
			pr_err("build bt_hs_imageview fail!\n");
	}

	mozart_smartui_hide_all();
	smartui_imageview_appear(bt_hs_background_imageview);

	return 1;
}

static struct imageview_struct *bt_connect_imageview;
static struct textview_struct *bt_connect_textview;
bool mozart_smartui_is_bt_connect_view(void)
{
	return !strcmp(owner, "bt_connect");
}

static int mozart_smartui_build_bt_connect_view(void)
{
	struct view_struct view = {
		.left = 70,
		.top = 80,
		.right = 170,
		.bottom = 180,
		.layer = top_layer,
		.align = left_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "bt_connect")) {
		smartui_clear_top(&global_background_imageview->v);

		bt_connect_imageview = smartui_imageview(&view);
		if (bt_connect_imageview == NULL)
			pr_err("build bt_connect_imageview fail!\n");

		view.left = 0;
		view.top = 190;
		view.right = 240;
		view.bottom = 220;
		view.layer = top_layer;
		view.align = center_align;
		bt_connect_textview = smartui_textview(&view);
		if (bt_connect_textview == NULL)
			pr_err("build bt_connect_textview fail!\n");

		strncpy(owner, "bt_connect", 64);
		return 1;
	}

	return 0;
}

static struct imageview_struct *bt_play_imageview;
static struct imageview_struct *bt_vol_imageview;
static struct textview_struct *bt_prompt_textview;
static struct barview_struct *bt_barview;
static int bt_barview_col_num;
bool mozart_smartui_is_bt_avk_view(void)
{
	return !strcmp(owner, "bt_avk");
}

static int mozart_smartui_build_bt_avk_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 40,
		.right = 240,
		.bottom = 80,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};
	int token[43] = {3, 7, 11, 14, 18, 22, 26, 30, 33, 37, 41, 45, 49,
			 52, 56, 60, 64, 68, 71, 75, 79, 83, 87, 90, 94, 98,
			 102, 106, 109, 113, 117, 121, 124, 128, 132, 136,
			 140, 144, 147, 151, 155, 159, -1};

	if (strcmp(owner, "bt_avk")) {
		smartui_clear_top(&global_background_imageview->v);

		bt_prompt_textview = smartui_textview(&view);
		if (bt_prompt_textview == NULL)
			pr_err("build bt_prompt_textview fail!\n");

		view.left = 105;
		view.top = 95;
		view.right = 137;
		view.bottom = 115;
		view.layer = top_layer;
		view.align = left_align;
		bt_play_imageview = smartui_imageview(&view);
		if (bt_play_imageview == NULL)
			pr_err("build bt_play_imageview fail!\n");

		view.left = 0;
		view.top = 266;
		view.right = 240;
		view.bottom = 306;
		view.layer = top_layer;
		view.align = left_align;
		bt_vol_imageview = smartui_imageview(&view);
		if (bt_vol_imageview == NULL)
			pr_err("build bt_vol_imageview fail!\n");

		view.left = 38;
		view.top = 150;
		view.right = 38 + 164;
		view.bottom = 150 + 61;
		view.layer = top_layer;
		view.align = center_align;
		bt_barview = smartui_barview(&view);
		if (bt_barview == NULL)
			pr_err("build bt_barview fail!\n");
		smartui_barview_set_pattern(bt_barview, SMARTUI_PATH"bt_bar.bmp", token);
		bt_barview_col_num = smartui_barview_get_col_num(bt_barview);

		strncpy(owner, "bt_avk", 64);
		return 1;
	}

	return 0;
}

static struct imageview_struct *linein_vol_imageview;
bool mozart_smartui_is_linein_view(void)
{
	return !strcmp(owner, "linein");
}

static int mozart_smartui_build_linein_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 185,
		.right = 240,
		.bottom = 225,
		.layer = top_layer,
		.align = left_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "linein")) {
		smartui_clear_top(&global_background_imageview->v);

		linein_vol_imageview = smartui_imageview(&view);
		if (linein_vol_imageview == NULL)
			pr_err("build linein_vol_imageview fail!\n");

		strncpy(owner, "linein", 64);
		return 1;
	}

	return 0;
}

static __attribute__((unused)) struct imageview_struct *xxx_prompt_imageview;
static __attribute__((unused)) struct textview_struct *xxx_prompt_textview;
bool mozart_smartui_is_xxx_view(void)
{
	return !strcmp(owner, "xxx");
}

static __attribute__((unused)) int mozart_smartui_build_xxx_view(void)
{
	if (strcmp(owner, "xxx")) {
		smartui_clear_top(&global_background_imageview->v);

		/* .bottom_view = &global_background_imageview->v, */

		strncpy(owner, "xxx", 64);
		return 1;
	}

	return 0;
}

/*******************************************************************************
 * misc
 *******************************************************************************/
static char *mozart_smartui_vol_image(void)
{
	int volume = mozart_volume_get();

	switch (volume) {
	case 0:
		return SMARTUI_PATH"vol_0.bmp";
	case 1 ... 10:
		return SMARTUI_PATH"vol_10.bmp";
	case 11 ... 20:
		return SMARTUI_PATH"vol_20.bmp";
	case 21 ... 30:
		return SMARTUI_PATH"vol_30.bmp";
	case 31 ... 40:
		return SMARTUI_PATH"vol_40.bmp";
	case 41 ... 50:
		return SMARTUI_PATH"vol_50.bmp";
	case 51 ... 60:
		return SMARTUI_PATH"vol_60.bmp";
	case 61 ... 70:
		return SMARTUI_PATH"vol_70.bmp";
	case 71 ... 80:
		return SMARTUI_PATH"vol_80.bmp";
	case 81 ... 90:
		return SMARTUI_PATH"vol_90.bmp";
	case 91 ... 100:
		return SMARTUI_PATH"vol_100.bmp";
	default:
		return SMARTUI_PATH"vol_50.bmp";
	}
}

void mozart_smartui_volume_update(void)
{
	struct imageview_struct *iv = NULL;

	pthread_mutex_lock(&mutex);

	if (mozart_smartui_is_atalk_view())
		iv = atalk_vol_imageview;
	else if (mozart_smartui_is_bt_avk_view())
		iv = bt_vol_imageview;
	else if (mozart_smartui_is_linein_view())
		iv = linein_vol_imageview;

	if (iv) {
		smartui_imageview_display(iv, mozart_smartui_vol_image());
		mozart_smartui_sync();
	}

	pthread_mutex_unlock(&mutex);
}

static char *mozart_smartui_wifi_image(void)
{
	if (__mozart_module_is_online())
		return SMARTUI_PATH"wifi_link_icon.bmp";
	else
		return SMARTUI_PATH"wifi_unlink_icon.bmp";
}

void mozart_smartui_wifi_update(void)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_display(global_wifi_imageview, mozart_smartui_wifi_image());
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_update_update(bool need)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_global_update_view();
	if (need)
		smartui_imageview_display(global_update_imageview, SMARTUI_PATH"update.bmp");
	else
		smartui_imageview_display(global_update_imageview, "");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_update_hide(bool hide)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_global_update_view();
	if (hide)
		smartui_imageview_hide(global_update_imageview);
	else
		smartui_imageview_appear(global_update_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

static char *battery_icon_str[][2] = {
	{
		SMARTUI_PATH"battery_10.bmp",
		SMARTUI_PATH"battery_chg_10.bmp",
	},
	{
		SMARTUI_PATH"battery_20.bmp",
		SMARTUI_PATH"battery_chg_20.bmp",
	},
	{
		SMARTUI_PATH"battery_30.bmp",
		SMARTUI_PATH"battery_chg_30.bmp",
	},
	{
		SMARTUI_PATH"battery_40.bmp",
		SMARTUI_PATH"battery_chg_40.bmp",
	},
	{
		SMARTUI_PATH"battery_50.bmp",
		SMARTUI_PATH"battery_chg_50.bmp",
	},
	{
		SMARTUI_PATH"battery_60.bmp",
		SMARTUI_PATH"battery_chg_60.bmp",
	},
	{
		SMARTUI_PATH"battery_70.bmp",
		SMARTUI_PATH"battery_chg_70.bmp",
	},
	{
		SMARTUI_PATH"battery_80.bmp",
		SMARTUI_PATH"battery_chg_80.bmp",
	},
	{
		SMARTUI_PATH"battery_90.bmp",
		SMARTUI_PATH"battery_chg_90.bmp",
	},
	{
		SMARTUI_PATH"battery_100.bmp",
		SMARTUI_PATH"battery_chg_100.bmp",
	},
};

static char *battery_last_icon;
static int battery_last_index = 9;
static char *mozart_smartui_battery_image(int capacity, int online)
{
	int battery_index;
	char *battery_icon;

	switch (capacity) {
	case 0 ... 15:
		battery_index = 0;
		break;
	case 16 ... 29:
		battery_index = 1;
		break;
	case 30 ... 39:
		battery_index = 2;
		break;
	case 40 ... 49:
		battery_index = 3;
		break;
	case 50 ... 59:
		battery_index = 4;
		break;
	case 60 ... 69:
		battery_index = 5;
		break;
	case 70 ... 79:
		battery_index = 6;
		break;
	case 80 ... 89:
		battery_index = 7;
		break;
	case 90 ... 96:
		battery_index = 8;
		break;
	case 97 ... 100:
		battery_index = 9;
		break;
	default:
		return battery_last_icon;
	}

	battery_icon = battery_icon_str[battery_index][online];

	battery_last_index = battery_index;
	battery_last_icon = battery_icon;

	return battery_icon;
}

void mozart_smartui_battery_update(int capacity, int online)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_display(
		global_battery_imageview,
		mozart_smartui_battery_image(capacity, online));
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

char *mozart_smartui_owner(void)
{
	return owner;
}

/*******************************************************************************
 * boot
 *******************************************************************************/
void mozart_smartui_startup(void)
{
	struct view_struct view = {
		.left = 12,
		.top = 6,
		.right = 90,
		.bottom = 28,
		.layer = top_layer,
		.align = center_align,
	};

	smartui_startup();
	system("echo 120 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");

	update_background_imageview = smartui_imageview(&global_background_view);
	if (update_background_imageview == NULL)
		pr_err("build update_background_imageview fail!\n");
	if (smartui_imageview_hide(update_background_imageview))
		pr_err("hide update_background_imageview fail!\n");

	asr_background_imageview = smartui_imageview(&global_background_view);
	if (asr_background_imageview == NULL)
		pr_err("build asr_background_imageview fail!\n");
	if (smartui_imageview_hide(asr_background_imageview))
		pr_err("hide asr_background_imageview fail!\n");

	bt_hs_background_imageview = smartui_imageview(&global_background_view);
	if (bt_hs_background_imageview == NULL)
		pr_err("build bt_hs_background_imageview fail!\n");
	if (smartui_imageview_hide(bt_hs_background_imageview))
		pr_err("hide bt_hs_background_imageview fail!\n");

	global_background_imageview = smartui_imageview(&global_background_view);
	if (global_background_imageview == NULL)
		pr_err("build global_background_imageview fail!\n");

	view.bottom_view = &global_background_imageview->v;
	global_info_textview = smartui_textview(&view);
	if (global_info_textview == NULL)
		pr_err("build global_info_textview fail!\n");
	smartui_textview_font_set(global_info_textview, HZK16);
	smartui_textview_set_global(global_info_textview);

	view.left = 90;
	view.top = 0;
	view.right = 90 + 28;
	view.bottom = 28;
	view.layer = top_layer;
	view.align = left_align;
	global_wifi_imageview = smartui_imageview(&view);
	if (global_wifi_imageview == NULL)
		pr_err("build global_wifi_imageview fail!\n");
	smartui_imageview_set_global(global_wifi_imageview);

	view.left = 174;
	view.top = 0;
	view.right = 234;
	view.bottom = 28;
	view.layer = top_layer;
	view.align = left_align;
	global_battery_imageview = smartui_imageview(&view);
	if (global_battery_imageview == NULL)
		pr_err("build global_battery_imageview fail!\n");
	smartui_imageview_set_global(global_battery_imageview);
}

void mozart_smartui_shutdown(void)
{
	system("echo 0 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");
	smartui_imageview_destory(asr_background_imageview);
	smartui_imageview_destory(global_background_imageview);
	smartui_shutdown();
}

void mozart_smartui_boot_build_display(char *s)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(boot_prompt_textview, s);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_display(char *s)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_boot_view()) {
		pr_err("isn't boot view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	if (s) {
		smartui_textview_display(boot_prompt_textview, s);
		mozart_smartui_sync();
	}

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");
	smartui_imageview_display(global_wifi_imageview, mozart_smartui_wifi_image());

	smartui_textview_display(boot_prompt_textview, "正在启动,请稍候");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_welcome(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(boot_prompt_textview, "  Hi ! 欢迎回来，播  放音乐电台");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_local(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(boot_prompt_textview,
				 "抱歉音箱似乎没有连上网络，为您播放缓存歌曲");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_factory_reset(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(boot_prompt_textview,
				 "音箱即将恢复出厂设置,请稍等");
	mozart_smartui_sync();

	/* pthread_mutex_unlock(&mutex); */
}

void mozart_smartui_boot_power_off(char *reason)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	if (reason)
		smartui_textview_display(boot_prompt_textview, reason);
	mozart_smartui_sync();

	/* pthread_mutex_unlock(&mutex); */
}

/*******************************************************************************
 * net
 *******************************************************************************/
void mozart_smartui_net_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_net_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(net_prompt_textview, "配置网络中");
	smartui_imageview_clear(net_result_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_net_receive_success(void)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_net_view()) {
		pr_err("isn't net view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(net_prompt_textview, "接收到WIFI配置");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_net_success(void)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_net_view()) {
		pr_err("isn't net view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(net_prompt_textview, "联网成功，正在为您连接网络服务");
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"ok.bmp");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_net_fail(char *reason)
{
	char str[128];

	if (reason)
		snprintf(str, 128, "联网未成功，请查看手机提示，检查后重试(%s)", reason);
	else
		snprintf(str, 128, "联网未成功，请查看手机提示，检查后重试");

	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_net_view()) {
		pr_err("isn't net view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(net_prompt_textview, str);
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"fail.bmp");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * atalk
 *******************************************************************************/
void mozart_smartui_atalk_prompt(char *prompt)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_atalk_view()) {
		pr_err("isn't atalk view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(global_info_textview, prompt);

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_atalk_play(char *vendor, char *title, char *artist, char *prompt)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_atalk_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"play_1.bmp");

	if (prompt)
		smartui_textview_display(global_info_textview, prompt);
	else
		smartui_textview_display(global_info_textview, "音乐电台");
	if (vendor)
		smartui_textview_display(atalk_vendor_textview, vendor);
	if (title)
		smartui_textview_display(atalk_title_textview, title);
	if (artist)
		smartui_textview_display(atalk_artist_textview, artist);

	smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	smartui_imageview_display(atalk_vol_imageview, mozart_smartui_vol_image());

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_atalk_toggle(bool play)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_atalk_view()) {
		pr_err("isn't atalk view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	if (play)
		smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	else
		smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"play_icon.bmp");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}


/*******************************************************************************
 * asr
 *******************************************************************************/
void mozart_smartui_asr_start(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_success.bmp");
	smartui_textview_display(asr_prompt_textview, "请吩咐");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_recognize(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_success.bmp");
	smartui_textview_display(asr_prompt_textview, "语音识别中");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_over(void)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_hide(asr_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_offline(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_fail.bmp");
	smartui_textview_display(asr_prompt_textview, "抱歉，音箱未联网，无法使用语音功能");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_success(char *s)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_success.bmp");
	smartui_textview_display(asr_prompt_textview, s);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_fail(char *s)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_fail.bmp");
	smartui_textview_display(asr_prompt_textview, s);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * bt_connect
 *******************************************************************************/
void mozart_smartui_bt_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_bt_connect_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"bt.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_imageview_display(bt_connect_imageview, SMARTUI_PATH"bt_icon.bmp");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_connecting(void)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_bt_connect_view()) {
		pr_err("isn't bt_connect view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(bt_connect_textview, "蓝牙连接中");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_connected(void)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_bt_connect_view()) {
		pr_err("isn't bt_connect view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_textview_display(bt_connect_textview, "蓝牙已连接");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_disconnect(void)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_bt_avk_view() && !mozart_smartui_is_bt_connect_view()) {
		pr_err("isn't bt_avk view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	if (mozart_smartui_build_bt_connect_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"bt.bmp");

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_imageview_display(bt_connect_imageview, SMARTUI_PATH"bt_icon.bmp");
	smartui_textview_display(bt_connect_textview, "蓝牙已断开");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * bt_hs
 *******************************************************************************/
void mozart_smartui_bt_hs(void)
{
	pthread_mutex_lock(&mutex);

	usleep(500 * 1000);
	mozart_smartui_build_bt_hs_view();

	smartui_imageview_display(bt_hs_background_imageview,  SMARTUI_PATH"bt.bmp");
	smartui_imageview_display(bt_hs_imageview, SMARTUI_PATH"bt_hs.bmp");
	smartui_textview_display(bt_hs_prompt_textview, "蓝牙通话中");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_hs_disconnect(void)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_hide(bt_hs_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * bt_avk
 *******************************************************************************/
int mozart_smartui_bt_get_play_col_num(void)
{
	return bt_barview_col_num;
}

void mozart_smartui_bt_play(void)
{
	int i, data[100] = {0};

	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_bt_avk_view()) {
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"bt_play.bmp");

		for (i = 0; i < bt_barview_col_num && i < 100; i++)
			data[i] = 100;
		data[i] = -1;

		smartui_barview_display(bt_barview, data);
	}

	smartui_textview_display(global_info_textview, "DS-1825");

	smartui_textview_display(bt_prompt_textview, "蓝牙播放中");
	smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	smartui_imageview_display(bt_vol_imageview, mozart_smartui_vol_image());
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_play_barview(int *data)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_bt_avk_view()) {
		pr_err("isn't bt_avk view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	smartui_barview_display(bt_barview, data);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_toggle(bool play)
{
	pthread_mutex_lock(&mutex);

	if (!mozart_smartui_is_bt_avk_view()) {
		pr_err("isn't bt_avk view\n");
		pthread_mutex_unlock(&mutex);
		return ;
	}

	if (play)
		smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	else
		smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"play_icon.bmp");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * linein
 *******************************************************************************/
void mozart_smartui_linein_play(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_linein_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"linein.bmp");

	smartui_textview_display(global_info_textview, "其它来源");

	smartui_imageview_display(linein_vol_imageview, mozart_smartui_vol_image());
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * update
 *******************************************************************************/
void mozart_smartui_update(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_update_view();
	smartui_imageview_display(update_background_imageview,  SMARTUI_PATH"1_240_320.bmp");
	smartui_textview_set_align(update_prompt0_textview, left_align);
	smartui_textview_display(update_prompt0_textview, "[下一首] 确认更新");
	smartui_textview_display(update_prompt1_textview, "[喜欢] 取消");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_update_start(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_update_view();
	smartui_imageview_display(update_background_imageview,  SMARTUI_PATH"1_240_320.bmp");
	smartui_textview_set_align(update_prompt0_textview, center_align);
	smartui_textview_display(update_prompt0_textview, "固件开始下载");
	smartui_textview_display(update_prompt1_textview, "");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_update_progress(int percent)
{
	char prog_str[48] = {0};

	pthread_mutex_lock(&mutex);

	sprintf(prog_str, "固件下载中...%d%%", percent);

	smartui_textview_set_align(update_prompt0_textview, center_align);
	smartui_textview_display(update_prompt0_textview, prog_str);
	smartui_textview_display(update_prompt1_textview, "");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_update_cancel(void)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_hide(update_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * xxx
 *******************************************************************************/
void mozart_smartui_xxx_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_xxx_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}
