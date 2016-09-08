#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "volume_interface.h"
#include "linklist_interface.h"
#include "smartui_interface.h"

#define SMARTUI_PATH "/usr/fs/usr/share/ui/"

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
static struct imageview_struct *global_background_imageview;

static struct textview_struct *boot_prompt_textview;
bool mozart_smartui_is_boot_view(void)
{
	return !strcmp(owner, "boot");
}

static int mozart_smartui_build_boot_view(void)
{
	struct view_struct view = {
		.left = 32,
		.top = 190,
		.right = 224,
		.bottom = 320,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "boot")) {
		smartui_clear_top(&global_background_imageview->v);

		boot_prompt_textview = smartui_textview(&view);
		if (boot_prompt_textview == NULL)
			fprintf(stderr, "build boot_prompt_textview fail!\n");
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
		.top = 240,
		.right = 224,
		.bottom = 320,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "net")) {
		smartui_clear_top(&global_background_imageview->v);

		net_prompt_textview = smartui_textview(&view);
		if (net_prompt_textview == NULL)
			fprintf(stderr, "build net_prompt_textview fail!\n");

		view.left = 70;
		view.top = 128;
		view.right = 170;
		view.bottom = 228;
		net_result_imageview = smartui_imageview(&view);
		if (net_result_imageview == NULL)
			fprintf(stderr, "build net_result_imageview fail!\n");

		strncpy(owner, "net", 64);
		return 1;
	}

	return 0;
}

static struct textview_struct *atalk_prompt_textview;
static struct textview_struct *atalk_vendor_textview;
static struct textview_struct *atalk_title_textview;
static struct textview_struct *atalk_artist_textview;
static struct imageview_struct *atalk_wifi_imageview;
static struct imageview_struct *atalk_battery_imageview;
static struct imageview_struct *atalk_play_imageview;
static struct imageview_struct *atalk_vol_imageview;
bool mozart_smartui_is_atalk_view(void)
{
	return !strcmp(owner, "atalk");
}

static int mozart_smartui_build_atalk_view(void)
{
	struct view_struct view = {
		.left = 12,
		.top = 6,
		.right = 90,
		.bottom = 28,
		.layer = top_layer,
		.align = left_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "atalk")) {
		smartui_clear_top(&global_background_imageview->v);

		atalk_prompt_textview = smartui_textview(&view);
		if (atalk_prompt_textview == NULL)
			fprintf(stderr, "build atalk_prompt_textview fail!\n");

		view.left = 0;
		view.top = 46;
		view.right = 240;
		view.bottom = 78;
		view.layer = top_layer;
		view.align = center_align;
		atalk_vendor_textview = smartui_textview(&view);
		if (atalk_vendor_textview == NULL)
			fprintf(stderr, "build atalk_vendor_textview fail!\n");

		view.left = 0;
		view.top = 175;
		view.right = 240;
		view.bottom = 207;
		view.layer = top_layer;
		view.align = center_align;
		atalk_title_textview = smartui_textview(&view);
		if (atalk_title_textview == NULL)
			fprintf(stderr, "build atalk_title_textview fail!\n");

		view.left = 0;
		view.top = 210;
		view.right = 240;
		view.bottom = 226;
		view.layer = top_layer;
		view.align = center_align;
		atalk_artist_textview = smartui_textview(&view);
		if (atalk_artist_textview == NULL)
			fprintf(stderr, "build atalk_artist_textview fail!\n");

		view.left = 90;
		view.top = 0;
		view.right = 130;
		view.bottom = 28;
		view.layer = top_layer;
		view.align = left_align;
		atalk_wifi_imageview = smartui_imageview(&view);
		if (atalk_wifi_imageview == NULL)
			fprintf(stderr, "build atalk_wifi_imageview fail!\n");

		view.left = 194;
		view.top = 0;
		view.right = 234;
		view.bottom = 28;
		view.layer = top_layer;
		view.align = left_align;
		atalk_battery_imageview = smartui_imageview(&view);
		if (atalk_battery_imageview == NULL)
			fprintf(stderr, "build atalk_battery_imageview fail!\n");

		view.left = 104;
		view.top = 139;
		view.right = 136;
		view.bottom = 159;
		view.layer = top_layer;
		view.align = left_align;
		atalk_play_imageview = smartui_imageview(&view);
		if (atalk_play_imageview == NULL)
			fprintf(stderr, "build atalk_play_imageview fail!\n");

		view.left = 0;
		view.top = 266;
		view.right = 240;
		view.bottom = 306;
		view.layer = top_layer;
		view.align = left_align;
		atalk_vol_imageview = smartui_imageview(&view);
		if (atalk_vol_imageview == NULL)
			fprintf(stderr, "build atalk_vol_imageview fail!\n");

		strncpy(owner, "atalk", 64);
		return 1;
	}

	return 0;
}

static struct imageview_struct *asr_background_imageview;
static struct textview_struct *asr_prompt_textview;
static int mozart_smartui_build_asr_view(void)
{
	struct view_struct view = {
		.left = 0,
		.top = 220,
		.right = 240,
		.bottom = 300,
		.layer = top_layer,
		.align = center_align,
		.bottom_view = &asr_background_imageview->v,
	};

	if (smartui_imageview_hide(global_background_imageview))
		fprintf(stderr, "hide global_background_imageview fail!\n");
	smartui_imageview_appear(asr_background_imageview);

	if (asr_prompt_textview == NULL)
		asr_prompt_textview = smartui_textview(&view);
	if (asr_prompt_textview == NULL)
		fprintf(stderr, "build asr_prompt_textview fail!\n");

	return 1;
}

static pthread_mutex_t display_delay_lock = PTHREAD_MUTEX_INITIALIZER;
static void *display_delay_func(void *args)
{
	sleep(3);
	smartui_imageview_hide(asr_background_imageview);
	smartui_imageview_appear(global_background_imageview);
	smartui_sync();

	return NULL;
}

static void mozart_smartui_asr_resume_view(bool now)
{
	pthread_t display_delay_pthread;

	if (now) {
		smartui_imageview_hide(asr_background_imageview);
		smartui_imageview_appear(global_background_imageview);
		smartui_sync();
	}

	pthread_mutex_lock(&display_delay_lock);

	if (pthread_create(&display_delay_pthread, NULL, display_delay_func, NULL) == -1) {
		fprintf(stderr, "create display delay fail\n");
		pthread_mutex_unlock(&display_delay_lock);
		return ;
	}
	pthread_detach(display_delay_pthread);
	pthread_mutex_unlock(&display_delay_lock);
}

static struct imageview_struct *bt_play_imageview;
static struct imageview_struct *bt_vol_imageview;
static struct textview_struct *bt_prompt_textview;
bool mozart_smartui_is_bt_view(void)
{
	return !strcmp(owner, "bt");
}

static __attribute__((unused)) int mozart_smartui_build_bt_view(void)
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

	if (strcmp(owner, "bt")) {
		smartui_clear_top(&global_background_imageview->v);

		bt_prompt_textview = smartui_textview(&view);
		if (bt_prompt_textview == NULL)
			fprintf(stderr, "build bt_prompt_textview fail!\n");

		view.left = 105;
		view.top = 95;
		view.right = 137;
		view.bottom = 115;
		view.layer = top_layer;
		view.align = left_align;
		bt_play_imageview = smartui_imageview(&view);
		if (bt_play_imageview == NULL)
			fprintf(stderr, "build bt_play_imageview fail!\n");

		view.left = 0;
		view.top = 266;
		view.right = 240;
		view.bottom = 306;
		view.layer = top_layer;
		view.align = left_align;
		bt_vol_imageview = smartui_imageview(&view);
		if (bt_vol_imageview == NULL)
			fprintf(stderr, "build bt_vol_imageview fail!\n");

		strncpy(owner, "bt", 64);
		return 1;
	}

	return 0;
}

static struct textview_struct *linein_prompt_textview;
static struct imageview_struct *linein_battery_imageview;
static struct imageview_struct *linein_vol_imageview;
static struct imageview_struct *linein_wifi_imageview;
bool mozart_smartui_is_linein_view(void)
{
	return !strcmp(owner, "linein");
}

static __attribute__((unused)) int mozart_smartui_build_linein_view(void)
{
	struct view_struct view = {
		.left = 12,
		.top = 6,
		.right = 90,
		.bottom = 28,
		.layer = top_layer,
		.align = left_align,
		.bottom_view = &global_background_imageview->v,
	};

	if (strcmp(owner, "linein")) {
		smartui_clear_top(&global_background_imageview->v);

		linein_prompt_textview = smartui_textview(&view);
		if (linein_prompt_textview == NULL)
			fprintf(stderr, "build linein_prompt_textview fail!\n");

		view.left = 90;
		view.top = 0;
		view.right = 130;
		view.bottom = 28;
		view.layer = top_layer;
		view.align = left_align;
		linein_wifi_imageview = smartui_imageview(&view);
		if (linein_wifi_imageview == NULL)
			fprintf(stderr, "build linein_wifi_imageview fail!\n");

		view.left = 0;
		view.top = 185;
		view.right = 240;
		view.bottom = 225;
		view.layer = top_layer;
		view.align = left_align;
		linein_vol_imageview = smartui_imageview(&view);
		if (linein_vol_imageview == NULL)
			fprintf(stderr, "build linein_vol_imageview fail!\n");

		view.left = 194;
		view.top = 0;
		view.right = 234;
		view.bottom = 28;
		view.layer = top_layer;
		view.align = left_align;
		linein_battery_imageview = smartui_imageview(&view);
		if (linein_battery_imageview == NULL)
			fprintf(stderr, "build linein_battery_imageview fail!\n");

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
	else if (mozart_smartui_is_bt_view())
		iv = bt_vol_imageview;
	else if (mozart_smartui_is_linein_view())
		iv = linein_vol_imageview;

	if (iv) {
		smartui_imageview_display(iv, mozart_smartui_vol_image());
		smartui_sync();
	}

	pthread_mutex_unlock(&mutex);
}

extern int atalk_online_flag;
static char *moszart_smartui_wifi_image(void)
{
	if (atalk_online_flag)
		return SMARTUI_PATH"wifi_link_icon.bmp";
	else
		return SMARTUI_PATH"wifi_unlink_icon.bmp";
}

/* TODO */
extern bool battery_status;
static char *mozart_smartui_battery_image(void)
{
	if (battery_status)
		return SMARTUI_PATH"battery_icon.bmp";
	else
		return SMARTUI_PATH"battery_charging_icon.bmp";
}

void mozart_smartui_battery_update(void)
{
	struct imageview_struct *iv = NULL;

	pthread_mutex_lock(&mutex);

	if (mozart_smartui_is_atalk_view())
		iv = atalk_battery_imageview;
	else if (mozart_smartui_is_linein_view())
		iv = linein_battery_imageview;

	if (iv) {
		smartui_imageview_display(iv, mozart_smartui_battery_image());
		smartui_sync();
	}

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
	smartui_startup();
	system("echo 120 > /sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness");

	asr_background_imageview = smartui_imageview(&global_background_view);
	if (asr_background_imageview == NULL)
		fprintf(stderr, "build asr_background_imageview fail!\n");
	if (smartui_imageview_hide(asr_background_imageview))
		fprintf(stderr, "hide asr_background_imageview fail!\n");

	global_background_imageview = smartui_imageview(&global_background_view);
	if (global_background_imageview == NULL)
		fprintf(stderr, "build global_background_imageview fail!\n");

	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(boot_prompt_textview, "正在启动,请稍候...");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_link(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(boot_prompt_textview, "正在为您连接网络服务");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_linked(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(boot_prompt_textview, "音箱已联网");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_welcome(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(boot_prompt_textview, "Hi! 欢迎回来，播放音乐电台");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_boot_local(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_boot_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"1_240_320.bmp");

	smartui_textview_display(boot_prompt_textview,
				 "抱歉音箱似乎没有连上网络，为您播放缓存歌曲");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * net
 *******************************************************************************/
void mozart_smartui_net_start(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_net_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	smartui_textview_display(net_prompt_textview, "配置网络中...");
	smartui_imageview_clear(net_result_imageview);
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_net_success(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_net_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	smartui_textview_display(net_prompt_textview, "联网成功，正在为您提供网络服务");
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"ok.bmp");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_net_fail(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_net_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"2_240_320.bmp");

	smartui_textview_display(net_prompt_textview, "联网未成功，请查看手机提示，检查后重试");
	smartui_imageview_display(net_result_imageview, SMARTUI_PATH"fail.bmp");
	smartui_sync();

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * atalk
 *******************************************************************************/
void mozart_smartui_atalk_prompt(char *prompt)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_atalk_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"play_1.bmp");

	smartui_textview_display(atalk_prompt_textview, prompt);

	smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_atalk_play(char *vendor, char *title, char *artist, char *prompt)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_atalk_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"play_1.bmp");

	if (prompt)
		smartui_textview_display(atalk_prompt_textview, prompt);
	else
		smartui_textview_display(atalk_prompt_textview, "音乐电台");
	if (vendor)
		smartui_textview_display(atalk_vendor_textview, vendor);
	if (title)
		smartui_textview_display(atalk_title_textview, title);
	if (artist)
		smartui_textview_display(atalk_artist_textview, artist);

	smartui_imageview_display(atalk_wifi_imageview, moszart_smartui_wifi_image());
	smartui_imageview_display(atalk_battery_imageview, mozart_smartui_battery_image());
	smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	smartui_imageview_display(atalk_vol_imageview, mozart_smartui_vol_image());

	smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_atalk_toggle(bool play)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_atalk_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"play_1.bmp");

	if (play)
		smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	else
		smartui_imageview_display(atalk_play_imageview, SMARTUI_PATH"play_icon.bmp");

	smartui_sync();
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

	smartui_textview_display(asr_prompt_textview, "语音识别中...");

	smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_offline(void)
{
	pthread_mutex_lock(&mutex);

	mozart_smartui_build_asr_view();
	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr_fail.bmp");

	smartui_textview_display(asr_prompt_textview, "抱歉，音箱未联网，无法使用语音功能");

	smartui_sync();
	mozart_smartui_asr_resume_view(false);

	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_success(void)
{
	pthread_mutex_lock(&mutex);
	mozart_smartui_asr_resume_view(true);
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_asr_fail(int index)
{
	pthread_mutex_lock(&mutex);

	smartui_imageview_display(asr_background_imageview,  SMARTUI_PATH"asr_fail.bmp");

	if (index == 2)
		smartui_textview_display(asr_prompt_textview, "网络有问题，请检查网络");
	else if (index == 3)
		smartui_textview_display(asr_prompt_textview, "服务忙，请稍候再试");
	else
		smartui_textview_display(asr_prompt_textview, "没听清请再试一次");
	smartui_sync();
	mozart_smartui_asr_resume_view(false);

	pthread_mutex_unlock(&mutex);
}

/*******************************************************************************
 * asr
 *******************************************************************************/
void mozart_smartui_bt_play(void)
{
	pthread_mutex_lock(&mutex);

	if (mozart_smartui_build_bt_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"bt_play.bmp");

	smartui_textview_display(bt_prompt_textview, "蓝 牙 播 放 中");
	smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	smartui_imageview_display(bt_vol_imageview, mozart_smartui_vol_image());

	smartui_sync();
	pthread_mutex_unlock(&mutex);
}

void mozart_smartui_bt_toggle(bool play)
{
	pthread_mutex_lock(&mutex);
	if (mozart_smartui_build_bt_view())
		smartui_imageview_display(global_background_imageview,  SMARTUI_PATH"bt_play.bmp");

	if (play)
		smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"pause_icon.bmp");
	else
		smartui_imageview_display(bt_play_imageview, SMARTUI_PATH"play_icon.bmp");

	smartui_sync();
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

	smartui_textview_display(linein_prompt_textview, "其它来源");
	smartui_imageview_display(linein_wifi_imageview, moszart_smartui_wifi_image());
	smartui_imageview_display(linein_battery_imageview, mozart_smartui_battery_image());
	smartui_imageview_display(linein_vol_imageview, mozart_smartui_vol_image());

	smartui_sync();
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

	smartui_sync();
	pthread_mutex_unlock(&mutex);
}
