#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#include "volume_interface.h"
#include "linklist_interface.h"
#include "power_interface.h"
#include "smartui_interface.h"
#include "mozart_app.h"
#include "vr-speech_interface.h"

#include "mozart_module.h"

#define SMARTUI_PATH "/mnt/sdcard/ui/"

#define SUPPORY_REFRESH_PIC

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
	.right = 176,
	.bottom = 220,
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
static struct imageview_struct *refresh_background_imageview;

//#define DIS_VERSION
#ifdef DIS_VERSION
#define VERSION "v0.1.2-160909-15:20"
static struct textview_struct *global_version_textview;
#endif

typedef enum refresh_pic_type_tag
{
	REFRESH_PIC_NET = 0x1,
	REFRESH_PIC_BT,
	REFRESH_PIC_ASR_SPEACK,
	REFRESH_PIC_ASR_RECONG,

	REFRESH_PIC_MAX,
	
} refresh_pic_type_t;

void mozart_smartui_stop_refreash_pic(void);
void mozart_smartui_stop_refreash_pic_show();
void mozart_smartui_start_refreash_pic(refresh_pic_type_t pic_type);
void mozart_smartui_refresh_pic_start(void);
void mozart_smartui_refresh_pic_stop(void);

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
		.left = 15,
		.top = 150,
		.right = 160,
		.bottom = 220,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "boot")) {
		smartui_clear_top(&global_background_imageview->v);

		boot_prompt_textview = smartui_textview(&view);
		if (boot_prompt_textview == NULL)
			pr_err("build boot_prompt_textview fail!\n");
		smartui_textview_font_set(boot_prompt_textview, HZK16);

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
		.left = 20,
		.top = 150,
		.right = 176 - 20,
		.bottom = 150 + 48 + 2,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "net")) {
		smartui_clear_top(&global_background_imageview->v);

		net_prompt_textview = smartui_textview(&view);
		if (net_prompt_textview == NULL)
			pr_err("build net_prompt_textview fail!\n");
		smartui_textview_font_set(net_prompt_textview, HZK16);

		view.left = 44;
		view.top = 87;
		view.right = 44+88;
		view.bottom = 87 + 65;
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
		.top = 30,
		.right = 176,
		.bottom = 30 + 16,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "atalk")) {
		smartui_clear_top(&global_background_imageview->v);

		atalk_vendor_textview = smartui_textview(&view);
		if (atalk_vendor_textview == NULL)
			pr_err("build atalk_vendor_textview fail!\n");
		smartui_textview_font_set(atalk_vendor_textview, HZK16);

		view.left = 10;
		view.top = 130;
		view.right = 166;
		view.bottom = 130 + 32 + 1;
		view.layer = top_layer;
		view.align = center_align;
		atalk_title_textview = smartui_textview(&view);
		if (atalk_title_textview == NULL)
			pr_err("build atalk_title_textview fail!\n");
		smartui_textview_font_set(atalk_title_textview, HZK16);

		view.left = 0;
		view.top = 163 + 5;
		view.right = 176;
		view.bottom = 168 + 16;
		view.layer = top_layer;
		view.align = center_align;
		atalk_artist_textview = smartui_textview(&view);
		if (atalk_artist_textview == NULL)
			pr_err("build atalk_artist_textview fail!\n");
		smartui_textview_font_set(atalk_artist_textview, HZK16);

		view.left = 77;
		view.top = 96;
		view.right = 77 + 22;
		view.bottom = 96 + 14;
		view.layer = top_layer;
		view.align = left_align;
		atalk_play_imageview = smartui_imageview(&view);
		if (atalk_play_imageview == NULL)
			pr_err("build atalk_play_imageview fail!\n");

		view.left = 4;
		view.top = 184;
		view.right = 4 + 168;
		view.bottom = 184 + 28;
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

#if 0
static struct textview_struct *asr_prompt_textview;
static struct imageview_struct *asr_icon_imageview;
static int mozart_smartui_build_asr_view(void)
{
	struct view_struct view = {
		.left = 10,
		.top = 150,
		.right = 166,
		.bottom = 150 + 48 + 2,
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
		smartui_textview_font_set(asr_prompt_textview, HZK16);
	}

	if (asr_icon_imageview == NULL) {
		view.left = 44;
		view.top = 67;
		view.right = 44 + 88;
		view.bottom = 67 + 91;
		view.layer = top_layer;
		view.align = center_align;
		asr_icon_imageview = smartui_imageview(&view);
		if (asr_icon_imageview == NULL)
			pr_err("build asr_icon_imageview fail!\n");
	}

	return 1;
}
#endif

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
		.left = 5,
		.top = 170,
		.right = 171,
		.bottom = 170 + 16,
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
		smartui_textview_font_set(update_prompt0_textview, HZK16);
	}

	if (update_prompt1_textview == NULL) {
		view.top = 186 + 1;
		view.bottom = 186 + 1 + 16;
		update_prompt1_textview = smartui_textview(&view);
		if (update_prompt1_textview == NULL)
			pr_err("build update_prompt1_textview fail!\n");
		smartui_textview_font_set(update_prompt1_textview, HZK16);
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
		.left = 38,
		.top = 30,
		.right = 38 + 100,
		.bottom = 30 + 100,
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
		view.top = 160;
		view.right = 176;
		view.bottom = 160 + 16;
		view.layer = top_layer;
		view.align = center_align;
		bt_connect_textview = smartui_textview(&view);
		if (bt_connect_textview == NULL)
			pr_err("build bt_connect_textview fail!\n");
		smartui_textview_font_set(bt_connect_textview, HZK16);

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
		.top = 30,
		.right = 176,
		.bottom = 30 + 16,
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
		smartui_textview_font_set(bt_prompt_textview, HZK16);

		view.left = 77;
		view.top = 66;
		view.right = 77 + 22;
		view.bottom = 66 + 14;
		view.layer = top_layer;
		view.align = center_align;
		bt_play_imageview = smartui_imageview(&view);
		if (bt_play_imageview == NULL)
			pr_err("build bt_play_imageview fail!\n");

		view.left = 0;
		view.top = 180;
		view.right = 176;
		view.bottom = 180 + 28;
		view.layer = top_layer;
		view.align = left_align;
		bt_vol_imageview = smartui_imageview(&view);
		if (bt_vol_imageview == NULL)
			pr_err("build bt_vol_imageview fail!\n");

		view.left = 6;
		view.top = 100;
		view.right = 6 + 164;
		view.bottom = 100 + 61;
		view.layer = top_layer;
		view.align = left_align;
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
		.right = 176,
		.bottom = 185 + 28,
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
		.left = 0,
		.top = 2,
		.right = 72,
		.bottom = 20,
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

	refresh_background_imageview = smartui_imageview(&global_background_view);
	if (refresh_background_imageview == NULL)
		pr_err("build refresh_background_imageview fail!\n");
	if (smartui_imageview_hide(refresh_background_imageview))
		pr_err("hide refresh_background_imageview fail!\n");

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

#ifdef DIS_VERSION

	view.left = 0; //114;
	view.top = 45;
	view.right = 176;
	view.bottom = 45 + 32 + 1;
	view.layer = top_layer;
	view.align = left_align;

	view.bottom_view = &global_background_imageview->v;
	global_version_textview = smartui_textview(&view);
	if (global_version_textview == NULL)
		pr_err("build global_version_textview fail!\n");
	smartui_textview_font_set(global_version_textview, HZK16);
	smartui_textview_set_global(global_version_textview);

#endif

	view.left = 78; //114;
	view.top = 0;
	view.right = 78 + 20;
	view.bottom = 20;
	view.layer = top_layer;
	view.align = left_align;
	global_wifi_imageview = smartui_imageview(&view);
	if (global_wifi_imageview == NULL)
		pr_err("build global_wifi_imageview fail!\n");
	smartui_imageview_set_global(global_wifi_imageview);

	view.left = 134;
	view.top = 0;
	view.right = 134 + 42;
	view.bottom = 20;
	view.layer = top_layer;
	view.align = left_align;
	global_battery_imageview = smartui_imageview(&view);
	if (global_battery_imageview == NULL)
		pr_err("build global_battery_imageview fail!\n");
	smartui_imageview_set_global(global_battery_imageview);
	
#ifdef SUPPORY_REFRESH_PIC
		mozart_smartui_refresh_pic_start();
#endif
}

void mozart_smartui_shutdown(void)
{
#ifdef SUPPORY_REFRESH_PIC
	mozart_smartui_refresh_pic_stop();
#endif
	
	system("echo 0 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");
	smartui_imageview_destory(asr_background_imageview);
	smartui_imageview_destory(global_background_imageview);
	smartui_shutdown();
}

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

void mozart_smartui_boot_build_display(char *s)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

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

	smartui_textview_display(global_info_textview, "DS-WB38");
	smartui_imageview_display(global_wifi_imageview, mozart_smartui_wifi_image());

#ifdef DIS_VERSION
	smartui_textview_display(global_version_textview, VERSION);
#endif

	smartui_textview_display(boot_prompt_textview, "正在启动,请稍候");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_welcome(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

	smartui_textview_display(boot_prompt_textview, "  Hi! 欢迎回来，播放音乐电台");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_local(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

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

	smartui_textview_display(global_info_textview, "DS-WB38");

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

	smartui_textview_display(global_info_textview, "DS-WB38");

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
#if 1
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_net_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

	smartui_textview_display(net_prompt_textview, "配置网络中");
	smartui_imageview_clear(net_result_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif

#ifndef SUPPORY_REFRESH_PIC
	mozart_smartui_start_refreash_pic(REFRESH_PIC_NET);
#endif
	
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
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"net_ok.bmp");
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
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"net_fail.bmp");
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
		smartui_textview_display(global_info_textview, "音  乐");
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

int asr_recog_mutl = 0;

void mozart_smartui_asr_start(void)
{
#if 0

	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	//smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_start.bmp");
	smartui_textview_display(asr_prompt_textview, "请吩咐");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif

#ifdef SUPPORY_REFRESH_PIC
	asr_recog_mutl++;
	if (asr_recog_mutl >= 2) 
		mozart_smartui_stop_refreash_pic();

	mozart_smartui_start_refreash_pic(REFRESH_PIC_ASR_SPEACK);
#endif
}



void mozart_smartui_asr_recognize(void)
{
#ifdef SUPPORY_REFRESH_PIC
	mozart_smartui_stop_refreash_pic();

	mozart_smartui_start_refreash_pic(REFRESH_PIC_ASR_RECONG);
#endif

#if 0
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_success.bmp");
	smartui_textview_display(asr_prompt_textview, "处理中,请稍后");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif
}

void mozart_smartui_asr_over(void)
{
#ifdef SUPPORY_REFRESH_PIC

	asr_recog_mutl = 0;

	mozart_smartui_stop_refreash_pic_show();

#endif

#if 0
	pthread_mutex_lock(&mutex);

	smartui_imageview_hide(asr_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif
}

void mozart_smartui_asr_offline(void)
{
	pr_debug("-----4--offline-----\n\n\n");

#ifdef SUPPORY_REFRESH_PIC
	asr_recog_mutl = 0;
	mozart_smartui_stop_refreash_pic_show();
#endif

#if 0
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_fail.bmp");
	smartui_textview_display(asr_prompt_textview, "抱歉，音箱未联网，无法使用语音功能");
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif
}

void mozart_smartui_asr_fail(char *s)
{
	pr_debug("-----5--fail-----\n\n\n");

#ifdef SUPPORY_REFRESH_PIC
	asr_recog_mutl = 0;
	mozart_smartui_stop_refreash_pic_show();
#endif

#if 0
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr.bmp");
	smartui_imageview_display(asr_icon_imageview,  SMARTUI_PATH"asr_fail.bmp");
	smartui_textview_display(asr_prompt_textview, error_text);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);
#endif
}

/*******************************************************************************
 * bt_connect
 *******************************************************************************/
void mozart_smartui_bt_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_bt_connect_view())
		smartui_imageview_display(global_background_imageview,	SMARTUI_PATH"bt.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

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
		smartui_imageview_display(global_background_imageview,	SMARTUI_PATH"bt.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");

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
		smartui_imageview_display(global_background_imageview,	SMARTUI_PATH"bt_play.bmp");

		for (i = 0; i < bt_barview_col_num && i < 100; i++)
			data[i] = 100;
		data[i] = -1;

		smartui_barview_display(bt_barview, data);
	}

	smartui_textview_display(global_info_textview, "DS-WB38");

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
 * standby
 *******************************************************************************/
static struct textview_struct *standby_time_textview;
static struct textview_struct *standby_date_textview;
bool mozart_smartui_is_standby_view(void)
{
	return !strcmp(owner, "standby");
}

static int mozart_smartui_build_standby_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 86,
		.right = 176,
		.bottom = 86 + 24,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};
		
	if (strcmp(owner, "standby")) {
		smartui_clear_top(&global_background_imageview->v);

		if (standby_time_textview == NULL) {
			standby_time_textview = smartui_textview(&view);
			if (standby_time_textview == NULL)
				pr_err("build standby_time_textview fail!\n");
			//smartui_textview_font_set(standby_time_textview, HZK16);
		}
		
		view.left = 0;
		view.top = 110;
		view.right = 176;
		view.bottom = 110 + 16;
		view.layer = top_layer;
		view.align = center_align;
		
		if (standby_date_textview == NULL) {
			standby_date_textview = smartui_textview(&view);
			if (standby_date_textview == NULL)
				pr_err("build standby_time_textview fail!\n");
			smartui_textview_font_set(standby_date_textview, HZK16);
		}

		strncpy(owner, "standby", 64);
		return 1;
	}

	return 0;
}

void mozart_smartui_standby_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_standby_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"standby_background.bmp");

	smartui_textview_display(global_info_textview, "DS-WB38");
	smartui_imageview_display(global_wifi_imageview, mozart_smartui_wifi_image());

#ifdef DIS_VERSION
	smartui_textview_display(global_version_textview, VERSION);
#endif

	smartui_textview_display(standby_time_textview, "10:22");
	smartui_textview_display(standby_date_textview, "7月22日 星期三");

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);
}
/*******************************************************************************
 * weather
 *******************************************************************************/
enum weather_encode {
	WEATHER_SUNNY = 0,
	WEATHER_CLOUD,
	WEATHER_RAIN,
	WEATHER_SNOW,
	WEATHER_FOG,
};

static char *weather_pic[] = {
	SMARTUI_PATH"weather_sunny.bmp",
	SMARTUI_PATH"weather_cloud.bmp",
	SMARTUI_PATH"weather_rain.bmp",
	SMARTUI_PATH"weather_snow.bmp",
	SMARTUI_PATH"weather_fog.bmp",
};

void print_hex(char *tmp)
{
	int i;
	for(i=0; i<strlen(tmp); i++)
		pr_debug("%x ", tmp[i]);
	pr_debug("\n");
}

char * substring(char *ch, int pos, int length)
{
	char *pch = ch;
	char *subch = (char *)calloc(sizeof(char), length+1);
	int i;

	pch += pos;

	for(i=0; i<length; i++)
		subch[i] = *(pch++);

	subch[length] = '\0';

	return subch;
}

static char *mozart_smartui_get_weather_pic(char * des)
{	
	int i;
	int pic_type = 0;
	char *world;
	
	for(i=0; i<strlen(des); i++) {
		if(i % 3 != 0)
			continue;
		
		world = substring(des, i, 3);
		
		if(!strcmp(world, "晴")) {
			pic_type = WEATHER_SUNNY;
			break;
		}
		if(!strcmp(world, "阴")) {
			pic_type = WEATHER_CLOUD;
			break;
		}
		if(!strcmp(world, "雨")) {
			pic_type = WEATHER_RAIN;
			break;
		}
		if(!strcmp(world, "雪")) {
			pic_type = WEATHER_SNOW;
			break;
		}
		if(!strcmp(world, "雾")) {
			pic_type = WEATHER_FOG;
			break;
		}
	}

	return weather_pic[pic_type];		
}

static struct textview_struct *weather_area_textview;
static struct textview_struct *weather_info_textview;
static struct textview_struct *weather_temp_textview;
bool mozart_smartui_is_weather_view(void)
{
	return !strcmp(owner, "weather");
}

static int mozart_smartui_build_weather_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 48,
		.right = 176,
		.bottom = 48 + 24,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};
		
	if (strcmp(owner, "weather")) {
		smartui_clear_top(&global_background_imageview->v);

		weather_area_textview = smartui_textview(&view);
		if (weather_area_textview == NULL)
			pr_err("build weather_area_textview fail!\n");
		//smartui_textview_font_set(weather_area_textview, HZK16);

		view.left = 10;
		view.top = 173;
		view.right = 166;
		view.bottom = 173 + 32 + 1;
		view.layer = top_layer;
		view.align = center_align;

		weather_info_textview = smartui_textview(&view);
		if (weather_info_textview == NULL)
			pr_err("build weather_info_textview fail!\n");
		smartui_textview_font_set(weather_info_textview, HZK16);
		
		view.left = 60;
		view.top = 120;
		view.right = 176;
		view.bottom = 120 + 16;
		view.layer = top_layer;
		view.align = center_align;

		weather_temp_textview = smartui_textview(&view);
		if (weather_temp_textview == NULL)
			pr_err("build weather_temp_textview fail!\n");
		smartui_textview_font_set(weather_temp_textview, HZK16);

		strncpy(owner, "weather", 64);
		return 1;
	}

	return 0;
}
void mozart_smartui_weather_start(weather_info recog)
{	
#ifdef SUPPORY_REFRESH_PIC
	//mozart_smartui_stop_refreash_pic();
#endif

	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_weather_view())
		smartui_imageview_display(global_background_imageview, mozart_smartui_get_weather_pic(recog.weather));
	
	smartui_textview_display(global_info_textview, "DS-WB38");
	smartui_imageview_display(global_wifi_imageview, mozart_smartui_wifi_image());

#ifdef DIS_VERSION
	smartui_textview_display(global_version_textview, VERSION);
#endif

	smartui_textview_display(weather_area_textview, recog.area);
	smartui_textview_display(weather_temp_textview, recog.temperature);
	smartui_textview_display(weather_info_textview, recog.wind);

	mozart_smartui_sync();
	pthread_mutex_unlock(&mutex);

}



#ifdef SUPPORY_REFRESH_PIC

sem_t sem_refresh_pic;
int gstop_refresh_flag = 1;
int gstop_refresh_flag_end = 0;
refresh_pic_type_t gpic_type = REFRESH_PIC_MAX;
char *gpic_name = NULL;

static struct imageview_struct *refresh_asr_icon_imageview;
static struct textview_struct 	 *refresh_asr_textview;
static struct imageview_struct *refresh_asr_recong_icon_imageview;
static struct imageview_struct *refresh_net_icon_imageview;
static struct imageview_struct *refresh_bt_icon_imageview;
static int mozart_smartui_build_refresh_pic_view(refresh_pic_type_t pic_type)
{
	struct view_struct view = {
		.layer = top_layer,
		.align = center_align,
		.left = 0,
		.top = 126,
		.right = 176,
		.bottom = 126 + 16,
		.align = center_align,
		.bottom_view = &refresh_background_imageview->v,
	};

	pr_debug("---------appear-------refresh_background_imageview--------\n");
	smartui_imageview_hide(global_background_imageview);
	smartui_imageview_appear(refresh_background_imageview);

	refresh_asr_textview = smartui_textview(&view);
	if (refresh_asr_textview == NULL)
		pr_err("build refresh_asr_textview fail!\n");
	smartui_textview_font_set(refresh_asr_textview, HZK16);

	switch (pic_type) {
		case REFRESH_PIC_NET: {
			view.left = 44;
			view.top = 67;
			view.right = 44 + 88;
			view.bottom = 67 + 59;
			view.align = center_align;
			
			if (refresh_net_icon_imageview == NULL) {
				refresh_net_icon_imageview = smartui_imageview(&view);
				if (refresh_net_icon_imageview == NULL)
					pr_err("build refresh_net_icon_imageview fail!\n");
				}
		}
		break;
		
		case REFRESH_PIC_BT: {
			if (refresh_bt_icon_imageview == NULL) {
				refresh_bt_icon_imageview = smartui_imageview(&view);
				if (refresh_bt_icon_imageview == NULL)
					pr_err("build refresh_bt_icon_imageview fail!\n");
				}
		}
		break;

		case REFRESH_PIC_ASR_SPEACK: {
			view.left = 44;
			view.top = 67;
			view.right = 44 + 88;
			view.bottom = 67 + 59;
			view.align = center_align;
				
			if (refresh_asr_icon_imageview == NULL) {
				refresh_asr_icon_imageview = smartui_imageview(&view);
				if (refresh_asr_icon_imageview == NULL)
					pr_err("build refresh_asr_icon_imageview fail!\n");
			}
		}
		break;
		case REFRESH_PIC_ASR_RECONG: {
			view.left = 62;
			view.top = 95;
			view.right = 62 + 51;
			view.bottom = 95 + 31;
			view.align = center_align;
			
			if (refresh_asr_recong_icon_imageview == NULL) {
				refresh_asr_recong_icon_imageview = smartui_imageview(&view);
				if (refresh_asr_recong_icon_imageview == NULL)
					pr_err("build refresh_asr_icon_imageview fail!\n");
			}
			
		}
		break;
		case REFRESH_PIC_MAX: 
		default:
		break;
	}

	return 1;
}

int g_refresh_pic_stop = 0;

void *create_refresh_pic_func(void *arg)
{
	int count = 0;
	int res;
	char tmp[50];
	
	res = sem_init(&sem_refresh_pic, 0, 0);  
    	if (res == -1) {  
 		perror("semaphore intitialization failed\n");  
        		exit(EXIT_FAILURE);  
    	} 
		
	while(!g_refresh_pic_stop) {
		pr_debug("------sem_wait----------\n");
		count = 0;
		sem_wait(&sem_refresh_pic);

		switch (gpic_type) {
		case REFRESH_PIC_NET: {
				pr_debug("------gpic_type---1-- \n");
				
				mozart_smartui_build_refresh_pic_view(REFRESH_PIC_NET);
				smartui_textview_display(refresh_asr_textview, "正在配置网络");
		}
		break;
		case REFRESH_PIC_BT :
		break;
		case REFRESH_PIC_ASR_SPEACK: {
				pr_debug("------gpic_type---2-- \n");

				mozart_smartui_build_refresh_pic_view(REFRESH_PIC_ASR_SPEACK);
				//smartui_textview_clear(refresh_asr_textview);
				smartui_textview_display(refresh_asr_textview, "请说话");
		}
		break;
		case REFRESH_PIC_ASR_RECONG: {
			pr_debug("------gpic_type---3-- \n");
						
			mozart_smartui_build_refresh_pic_view(REFRESH_PIC_ASR_RECONG);
			//smartui_textview_clear(refresh_asr_textview);
			smartui_textview_display(refresh_asr_textview, "识别中");
		}
		break;
		case REFRESH_PIC_MAX :
		default :
		break;
		}
		
		while (!gstop_refresh_flag) {
			
			pthread_mutex_lock(&mutex);

			if (count++ >= 5)
				count = 1;
			
			sprintf (tmp, "%s%s%02d.bmp", SMARTUI_PATH, gpic_name, count);

			switch (gpic_type) {
			case REFRESH_PIC_NET: 
				smartui_imageview_display(refresh_net_icon_imageview, tmp);
			break;
			case REFRESH_PIC_ASR_SPEACK: 
				smartui_imageview_display(refresh_asr_icon_imageview, tmp);
			break;
			case REFRESH_PIC_ASR_RECONG: 
				smartui_imageview_display(refresh_asr_recong_icon_imageview, tmp);
			break;
			case REFRESH_PIC_BT: 
			break;
			case REFRESH_PIC_MAX: 
			default:
			break;
			}

			mozart_smartui_sync();
			
			pthread_mutex_unlock(&mutex);
			//usleep(100 * 1000);
			usleep(10 * 1000);
		}

		smartui_textview_clear(refresh_asr_textview);
		smartui_imageview_clear(refresh_asr_icon_imageview);
		smartui_imageview_clear(refresh_asr_recong_icon_imageview);
		
		gstop_refresh_flag_end = 1;
		pr_debug("----after----while(!gstop_refresh_flag)------gstop_refresh_flag_end: %d \n", gstop_refresh_flag_end);
		usleep(1 * 1000);
	}
	g_refresh_pic_stop = 0;
	return NULL;
}


void mozart_smartui_refresh_pic_start(void)
{
	pthread_t refresh_pic;
	
	g_refresh_pic_stop = 0;

	if (pthread_create(&refresh_pic, NULL, create_refresh_pic_func, NULL) ) 
	{
		pr_err("create_smartui_thread pthread_create error: %s \n", strerror(errno));
	}

	if (pthread_detach(refresh_pic) ) 
	{
		pr_err("create_smartui_thread pthread_create error: %s \n", strerror(errno));
	}
}

void mozart_smartui_refresh_pic_stop(void)
{
	int timeout = 0;
	
	g_refresh_pic_stop = 1;
	
	while(g_refresh_pic_stop == 0) {
		usleep(1 * 1000);

		if (timeout ++ > 5000)
			break;		
	}
}

void mozart_smartui_start_refreash_pic(refresh_pic_type_t pic_type)
{
	switch (pic_type) {
		case REFRESH_PIC_NET: {
			gpic_name = "A";
			gpic_type = REFRESH_PIC_NET;
		}
		break;

		case REFRESH_PIC_BT: {
			gpic_name = "A";
			gpic_type = REFRESH_PIC_BT;
		}
		break;

		case REFRESH_PIC_ASR_SPEACK: {
			gpic_name = "B";
			gpic_type = REFRESH_PIC_ASR_SPEACK;
		}
		break;
		case REFRESH_PIC_ASR_RECONG: {
			gpic_name = "D";
			gpic_type = REFRESH_PIC_ASR_RECONG;
		}
		break;
		case REFRESH_PIC_MAX: 
		default:
		break;
	}

	gstop_refresh_flag_end = 0;
	gstop_refresh_flag = 0;
	pr_debug("------sem_post-------\n");
	sem_post(&sem_refresh_pic);
	usleep(10 * 1000);
	
}

void mozart_smartui_stop_refreash_pic(void)
{
	gstop_refresh_flag = 1;
	int timeout = 0;

	pr_debug("----stop----while(!gstop_refresh_flag_end)--------gstop_refresh_flag_end: %d \n", gstop_refresh_flag_end);
	while(!gstop_refresh_flag_end) {
		usleep(500);
		if (timeout++ > 500)
			break;
	}
	gstop_refresh_flag_end = 0;	
}

void mozart_smartui_stop_refreash_pic_show(void)
{
	//sem_wait(&sem_refresh_pic);
	int timeout = 0;

	
	gstop_refresh_flag = 1;

	pr_debug("----stop----while(!gstop_refresh_flag_end)--------gstop_refresh_flag_end: %d \n", gstop_refresh_flag_end);
	while(!gstop_refresh_flag_end) {
		usleep(500);
		if (timeout++ > 500)
			break;
	}
	gstop_refresh_flag_end = 0;

	pr_debug("---------smartui_imageview_appear(global_background_imageview)---------\n");
	pthread_mutex_lock(&mutex);
	smartui_imageview_hide(refresh_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	mozart_smartui_sync();

	pthread_mutex_unlock(&mutex);

	pr_debug("-----exit----asr UI-------\n");
}

#endif


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






