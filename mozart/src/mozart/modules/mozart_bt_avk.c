#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "bluetooth_interface.h"
#include "utils_interface.h"
#include "mozart_config.h"
#include "tips_interface.h"
#include "sharememory_interface.h"
#include "resample_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_linein.h"
#include "mozart_prompt_tone.h"
#include "mozart_bt_avk_fft.h"

#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#endif

#include "ini_interface.h"
#include "volume_interface.h"

#if (SUPPORT_WEBRTC == 1)
#include "webrtc_aec.h"
bt_aec_callback bt_ac;
#endif

#ifndef MOZART_RELEASE
#define MOZART_BT_AVK_DEBUG
#else
#define MOZART_RELEASE_NAME
#endif

#ifdef MOZART_BT_AVK_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[BT_AVK] %s: "fmt, __func__, ##args)
#else  /* MOZART_BT_AVK_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_BT_AVK_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[BT_AVK] [Error] %s: "fmt, __func__, ##args)

#define SUPPORT_BSA_AVK_RESAMPLE		1

#if (SUPPORT_BSA_AVK_RESAMPLE == 1)
static af_resample_t* avk_resample_s;
static avk_resample_init_data avk_resample_data = {
	.resample_rate = 0,
	.resample_bits = 0,
	.resample_channel = 0,
	.resample_enable = 0,
	.mozart_avk_resample_data_cback = NULL,
};
#endif

#define SUPPORT_BSA_AEC_RESAMPLE 1
#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
static af_resample_t* hs_resample_s;
static hs_sample_init_data hs_sample_data = {0};
static hs_resample_init_data hs_resample_data = {
	.resample_rate = 0,
	.resample_bits = 0,
	.resample_channel = 0,
	.resample_enable = 0,
	.mozart_hs_resample_data_cback = NULL,
};

static af_resample_t* aec_resample_s;
static bt_aec_sample_init_data bt_aec_sdata = {0};
static bt_aec_resample_init_data bt_aec_rdata = {
	.resample_rate = 0,
	.resample_bits = 0,
	.resample_channel = 0,
	.resample_enable = 0,
	.aec_resample_data_cback = NULL,
};
#endif
//static bool bt_avk_init;
/*******************************************************************************
 * Function
 *******************************************************************************/
static void bt_info_init(bt_init_info *bt_info, char *bt_name)
{
	bt_info->bt_name = bt_name;
	bt_info->discoverable = 1;
	bt_info->connectable = 1;
	memset(bt_info->out_bd_addr, 0, sizeof(bt_info->out_bd_addr));
}

#if (SUPPORT_BSA_AVK_RESAMPLE == 1)
static enum avk_data_stage {
	avk_data_stage_wait = 0,
	avk_data_stage_display,
} avk_data_stage;
static char avk_data[4096];
static int avk_data_offset;


#define APP_AVK_VOLUME_MAX	17

static UINT8 avk_volume_set_dsp[17] = {0, 6, 12, 18, 25, 31, 37, 43, 50,
				       56, 62, 68, 75, 81, 87, 93, 100};
static UINT8 avk_volume_set_phone[17] = {0, 15, 23, 31, 39, 47, 55, 63, 71,
					 79, 87, 95, 103, 111, 113, 125, 127};
static int bt_avk_volume_dsp2phone(int *volume)
{
	int i = 0;

	for (i = 0; i < APP_AVK_VOLUME_MAX - 1; i++)
		if (*volume >= avk_volume_set_dsp[i] && *volume < avk_volume_set_dsp[i + 1])
			break;

	*volume = avk_volume_set_dsp[i];
	printf("%s: dsp(%d) => phone(%d)\n", __func__, *volume, avk_volume_set_phone[i]);

	return avk_volume_set_phone[i];
}

static void bt_avk_module_volume_up(struct mozart_module_struct *self)
{
	int i = 0;
	char vol_buf[8] = {};
	int vol = 0, phone_vol;

	if (mozart_ini_getkey("/usr/data/system.ini", "volume", "bt_music", vol_buf)) {
		printf("failed to parse /usr/data/system.ini, set BT volume to 63.\n");
		vol = 63;
	} else {
		vol = atoi(vol_buf);
	}

	if (vol >= 100)
		printf("bt volume has maximum!\n");

	for (i = 0; i < APP_AVK_VOLUME_MAX; i++)
		if (avk_volume_set_dsp[i] > vol)
			break;

	if (i >= APP_AVK_VOLUME_MAX) {
		printf("failed to get music volume %d from avk_volume_set_dsp\n", vol);
		return;
	}

	vol = avk_volume_set_dsp[i];
	phone_vol = bt_avk_volume_dsp2phone(&vol);
	mozart_volume_set(vol, BT_MUSIC_VOLUME);
	mozart_bluetooth_avk_set_volume_up(phone_vol);
}

static void bt_avk_module_volume_down(struct mozart_module_struct *self)
{

	int i = 0;
	char vol_buf[8] = {};
	int vol = 0, phone_vol;

	if (mozart_ini_getkey("/usr/data/system.ini", "volume", "bt_music", vol_buf)) {
		printf("failed to parse /usr/data/system.ini, set BT volume to 63.\n");
		vol = 63;
	} else {
		vol = atoi(vol_buf);
	}

	if (vol < 0)
		printf("bt volume has minimum!\n");

	for (i = (APP_AVK_VOLUME_MAX - 1); i >= 0; i--)
		if (avk_volume_set_dsp[i] < vol)
			break;

	if (i >= APP_AVK_VOLUME_MAX) {
		printf("failed to get music volume %d from avk_volume_set_dsp\n", vol);
		return;
	}

	vol = avk_volume_set_dsp[i];
	phone_vol = bt_avk_volume_dsp2phone(&vol);
	mozart_volume_set(vol, BT_MUSIC_VOLUME);
	mozart_bluetooth_avk_set_volume_up(phone_vol);
}

#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
static int avk_resample_outlen_max = 0;
#endif
static void mozart_avk_resample_data_callback(avk_callback_msg *avk_msg)
{
	/* int offset; */
	int len = avk_msg->in_len;

	if (avk_data_stage == avk_data_stage_wait) {
		if (avk_data_offset + len < 4096) {
			memcpy((char *)avk_data + avk_data_offset, avk_msg->in_buffer, len);
			avk_data_offset += len;
		} else {
			memcpy((char *)avk_data + avk_data_offset, avk_msg->in_buffer, 4096 - avk_data_offset);
			avk_data_offset = 0;
			avk_data_stage = avk_data_stage_display;
		}
	}

#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
	int resample_outlen = mozart_resample_get_outlen(avk_msg->in_len, avk_msg->sample_rate,
							 avk_msg->sample_channel, avk_msg->sample_bits,
							 avk_resample_data.resample_rate);

	if (avk_msg->sample_rate == 48000) {
		avk_msg->out_len = avk_msg->in_len;
		avk_msg->out_buffer = avk_msg->in_buffer;
		return;
	}
	if (avk_msg->out_buffer == NULL) {
		printf("avk_msg->out_buffer == NULL !\n");
		avk_resample_outlen_max = resample_outlen;
		avk_msg->out_buffer = malloc(resample_outlen);
		avk_msg->out_len = resample_outlen;
	} else {
		if (resample_outlen > avk_resample_outlen_max) {
			printf("realloc !\n");
			avk_resample_outlen_max = resample_outlen;
			avk_msg->out_buffer = realloc(avk_msg->out_buffer, resample_outlen);
			avk_msg->out_len = resample_outlen;
		} else {
			avk_msg->out_len = resample_outlen;
		}
	}
	avk_msg->out_len = mozart_resample(avk_resample_s, avk_msg->sample_channel,
					   avk_msg->sample_bits, avk_msg->in_buffer,
					   avk_msg->in_len, avk_msg->out_buffer);
#else
	avk_msg->out_len = len;
	avk_msg->out_buffer = avk_msg->in_buffer;
#endif
}

int is_bt_avk_running = false;

static void *bt_avk_display_handler(void *args)
{
	pthread_detach(pthread_self());
	int i, col_num, data[100];

	while (is_bt_avk_running) {
		if (avk_data_stage == avk_data_stage_display) {
			col_num = mozart_smartui_bt_get_play_col_num();
			mozart_bt_avk_compute_height((short *)avk_data, (int *)data);
			for (i = 0; i < col_num && i < 100; i++)
				data[i] <<= 1;
			mozart_smartui_bt_play_barview(data);
			avk_data_stage = avk_data_stage_wait;
		}
		/* 44100, 16bit, 2 channel, 176400 byte/s
		 * 2048byte need 11.6ms
		 */
		usleep(20 * 1000);
	}

	return NULL;
}
#endif

#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
static char *hs_buf;
static int hs_buflen_max = 0;
static int hs_channel_outlen_max = 0;
static void mozart_hs_resample_data_callback(hs_resample_msg *hs_rmsg)
{
	int channel_outlen;
	int resample_outlen = mozart_resample_get_outlen(hs_rmsg->in_len,
							 hs_sample_data.sample_rate,
							 hs_sample_data.sample_channel,
							 hs_sample_data.sample_bits,
							 hs_resample_data.resample_rate);
	/* 8k -> 48k */
	if (hs_buf == NULL) {
		printf("hs_buf == NULL !\n");
		hs_buflen_max = resample_outlen;
		hs_buf = malloc(resample_outlen);
	} else {
		if (resample_outlen > hs_buflen_max) {
			printf("realloc !\n");
			hs_buflen_max = resample_outlen;
			hs_buf = realloc(hs_buf, resample_outlen);
		}
	}

	resample_outlen = mozart_resample(hs_resample_s, hs_sample_data.sample_channel, hs_sample_data.sample_bits,
					  hs_rmsg->in_buffer, hs_rmsg->in_len, hs_buf);

	/* 1channel -> 2channel */
	channel_outlen = resample_outlen * 2;
	if (hs_rmsg->out_buffer == NULL) {
		printf("hs_rmsg->out_buffer == NULL !\n");
		hs_channel_outlen_max = channel_outlen;
		hs_rmsg->out_buffer = malloc(channel_outlen);
		hs_rmsg->out_len = channel_outlen;
	} else {
		if (channel_outlen > hs_channel_outlen_max) {
			printf("realloc !\n");
			hs_channel_outlen_max = channel_outlen;
			hs_rmsg->out_buffer = realloc(hs_rmsg->out_buffer, channel_outlen);
			hs_rmsg->out_len = channel_outlen;
		} else {
			hs_rmsg->out_len = channel_outlen;
		}
	}
	int16_t* tin = (int16_t *)hs_buf;
	int i = channel_outlen / 4;
	int16_t* tout = (int16_t *)hs_rmsg->out_buffer;
	while (i--) {
		*tout = *tin;
		*(tout + 1) = *tin;

		tin += 1;
		tout += 2;
	}
}

static char *aec_buf;
static void mozart_bt_aec_resample_data_callback(bt_aec_resample_msg *bt_aec_rmsg)
{
	int channel_outlen;
	int resample_outlen;

	/* 48k -> 8k */
	if (aec_buf == NULL) {
		/* 48k, 2channel, 16bit */
		resample_outlen = mozart_resample_get_outlen(bt_aec_rmsg->in_len,
								 bt_aec_sdata.sample_rate,
								 bt_aec_sdata.sample_channel,
								 bt_aec_sdata.sample_bits,
								 bt_aec_rdata.resample_rate);

		aec_buf = malloc(resample_outlen);
	}

	resample_outlen = mozart_resample(aec_resample_s, bt_aec_sdata.sample_channel, bt_aec_sdata.sample_bits,
		bt_aec_rmsg->in_buffer, bt_aec_rmsg->in_len, aec_buf);

	/* 2channel -> 1channel */
	channel_outlen = resample_outlen / 2;
	if (bt_aec_rmsg->out_buffer == NULL) {
		printf("bt_aec_rmsg->out_buffer == NULL !\n");
		bt_aec_rmsg->out_buffer = malloc(channel_outlen);
		bt_aec_rmsg->out_len = channel_outlen;
	}

	int16_t* tin = (int16_t *)aec_buf;
	int i = channel_outlen / 2;
	int16_t* tout = (int16_t *)bt_aec_rmsg->out_buffer;
	while (i--) {
		*tout = *tin / 2 + *(tin + 1) / 2;
		tin += 2;
		tout += 1;
	}
}
#endif

static void *thr_fn(void *args)
{
	pthread_detach(pthread_self());
	int i = 0;
	bt_init_info bt_info;
	char bt_name[64] = {};
	char bt_avk_name[25] = "/var/run/bt-avk-fifo";
	char bt_socket_name[30] = "/var/run/bt-daemon-socket";
	__attribute__((unused)) char mac[] = "00:11:22:33:44:55";

	for (i = 0; i < 100; i++) {
		if (!access(bt_socket_name, 0) && !access(bt_avk_name, 0))
			break;
		else
			usleep(50000);
	}

	if (access(bt_socket_name, 0) || access(bt_avk_name, 0)) {
		printf("%s or %s not exists, please check !!\n",
				bt_avk_name, bt_socket_name);
		goto err_exit;
	}

#ifdef MOZART_RELEASE_NAME
	memset(bt_name, 0, sizeof(bt_name));
	sprintf(bt_name, BT_NAME);
#else
	memset(mac, 0, sizeof(mac));
	memset(bt_name, 0, sizeof(bt_name));
	get_mac_addr("wlan0", mac, "");

	strcat(bt_name, "SmartAudio-");
	strcat(bt_name, mac+4);
#endif

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

	mozart_bluetooth_set_visibility(true, true);
//	mozart_bluetooth_set_visibility(false, false);

#if (SUPPORT_BSA_AVK_RESAMPLE == 1)
#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
	avk_resample_data.resample_rate = 48000;
	avk_resample_s = mozart_resample_init(44100, 2, 2, avk_resample_data.resample_rate);
	if (avk_resample_s == NULL) {
		printf("ERROR: mozart resample inti failed\n");
		goto err_exit;
	}
#else
	avk_resample_data.resample_rate = BSA_AVK_ORI_SAMPLE;
#endif
	avk_resample_data.resample_channel = BSA_AVK_ORI_SAMPLE;
	avk_resample_data.resample_bits = BSA_AVK_ORI_SAMPLE;
	avk_resample_data.resample_enable = 1;
	avk_resample_data.mozart_avk_resample_data_cback = mozart_avk_resample_data_callback;
	mozart_bluetooth_avk_set_resampledata_callback(&avk_resample_data);
#endif /* SUPPORT_BSA_AVK_RESAMPLE */

#if (SUPPORT_BSA_AEC_RESAMPLE == 1)
	mozart_hs_get_default_sampledata(&hs_sample_data);

	/* play /dev/dsp */
	hs_resample_data.resample_rate = 48000;
	hs_resample_s = mozart_resample_init(hs_sample_data.sample_rate, hs_sample_data.sample_channel, hs_sample_data.sample_bits>>3, hs_resample_data.resample_rate);
	if(hs_resample_s == NULL){
		printf("ERROR: mozart resample inti failed\n");
		goto err_exit;
	}
	hs_resample_data.resample_bits = hs_sample_data.sample_bits;
	hs_resample_data.resample_channel = 2;
	hs_resample_data.resample_enable = 1;
	hs_resample_data.mozart_hs_resample_data_cback = mozart_hs_resample_data_callback;

	mozart_bluetooth_hs_set_resampledata_callback(&hs_resample_data);

	mozart_aec_get_bt_default_sampledata(&bt_aec_sdata);
	/* record dmic */
	bt_aec_rdata.resample_rate = bt_aec_sdata.sample_rate;
	bt_aec_rdata.resample_bits = bt_aec_sdata.sample_bits;
	bt_aec_rdata.resample_channel = bt_aec_sdata.sample_channel;
	/* record /dev/dsp */
	bt_aec_sdata.sample_rate = hs_resample_data.resample_rate;
	bt_aec_sdata.sample_bits = hs_resample_data.resample_bits;
	bt_aec_sdata.sample_channel = hs_resample_data.resample_channel;
	aec_resample_s = mozart_resample_init(bt_aec_sdata.sample_rate, bt_aec_sdata.sample_channel, bt_aec_sdata.sample_bits>>3, bt_aec_rdata.resample_rate);
	if(aec_resample_s == NULL) {
		printf("ERROR: mozart resample inti failed\n");
		goto err_exit;
	}

	bt_aec_rdata.resample_enable = 1;
	bt_aec_rdata.aec_resample_data_cback = mozart_bt_aec_resample_data_callback;
	mozart_aec_set_bt_resampledata_callback(&bt_aec_rdata);
#endif	/* SUPPORT_BSA_AEC_RESAMPLE */

err_exit :
	return NULL;
}

static int start_bt(void)
{
	int err;
	pthread_t p_tid;
	pthread_t bt_avk_display_thread;

	is_bt_avk_running = true;
	printf("------------------------------start bt\n\n\n");
#if 0
	if (!bt_avk_init) {
		system("/usr/fs/etc/init.d/S04bsa.sh start");
		bt_avk_init = true;
	}
#endif

	system("/usr/fs/etc/init.d/S04bsa.sh start");

#if (SUPPORT_WEBRTC == 1)
	bt_ac.aec_init = ingenic_apm_init;
	bt_ac.aec_destroy = ingenic_apm_destroy;
	bt_ac.aec_enable = webrtc_aec_enable;
	bt_ac.aec_get_buffer_length = webrtc_aec_get_buffer_length;
	bt_ac.aec_calculate = (void (*)(void *, void *, void *, unsigned int))webrtc_aec_calculate;

	mozart_aec_callback(&bt_ac);
#else
	mozart_aec_callback(NULL);
#endif

	err = pthread_create(&p_tid, NULL, thr_fn, NULL);
	if (err != 0)
		printf("can't create thread: %s\n", strerror(err));

	//pthread_detach(p_tid);

	err = pthread_create(&bt_avk_display_thread, NULL, bt_avk_display_handler, NULL);
	if (err != 0)
		pr_err("Create bt_avk_display pthread: %s\n", strerror(err));
	//pthread_detach(bt_avk_display_thread);

	return 0;
}

static int stop_bt(void)
{

	printf("------------------------------stop bt\n\n\n");
#if 0
	system("bt_disable");
#else
	/* TODO: DO nothing now. */
	mozart_bluetooth_disconnect(USE_HS_AVK);
	mozart_bluetooth_hs_stop_service();
	mozart_bluetooth_avk_stop_service();
	mozart_bluetooth_uninit();

	is_bt_avk_running = false;
	usleep(10*1000);
	system("/usr/fs/etc/init.d/S04bsa.sh stop");
#endif

	return 0;
}

static int bt_avk_resume_handler(void)
{
	if (!mozart_bluetooth_avk_get_play_state())
		mozart_bluetooth_avk_play_pause();
	else
		pr_debug("avk state is playing\n");

	return 0;
}

static int bt_avk_pause_handler(void)
{
	if (mozart_bluetooth_avk_get_play_state())
		mozart_bluetooth_avk_play_pause();
	else
		pr_debug("avk state is pause\n");

	return 0;
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int bt_avk_module_start(struct mozart_module_struct *self)
{
	int ret, timeout = 5;

	mozart_bt_avk_init_fft();
	start_bt();
	mozart_smartui_bt_start();

	if (self->module_change)
		__mozart_prompt_tone_key_sync("atalk_swich_false_5");

	do {
		ret = mozart_bluetooth_auto_reconnect(USE_HS_AVK, 0);
		if (timeout-- <= 0)
			break;
	} while (ret);

	self->player_state = player_state_idle;

	return 0;
}

static int bt_avk_module_run(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play) {
		mozart_bluetooth_avk_open_dsp(0);
		return bt_avk_resume_handler();
	} else {
		return 0;
	}
}

static int bt_avk_module_suspend(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play) {
		mozart_bluetooth_avk_close_dsp(0);
		return bt_avk_pause_handler();
	} else {
		return 0;
	}
}

static int bt_avk_module_stop(struct mozart_module_struct *self)
{
	self->player_state = player_state_idle;
	mozart_bluetooth_avk_close_dsp(0);
	mozart_bluetooth_disconnect(USE_HS_AVK);
	mozart_bluetooth_set_visibility(false, false);

	stop_bt();
	//mozart_speech_startup(VOICE_WAKEUP);

	return 0;
}

static void bt_avk_module_previous_song(struct mozart_module_struct *self)
{
	mozart_bluetooth_avk_prev_music();
}

static void bt_avk_module_next_song(struct mozart_module_struct *self)
{
	mozart_bluetooth_avk_next_music();
}

static void bt_avk_module_resume_pause(struct mozart_module_struct *self)
{
	bsa_link_status link = mozart_bluetooth_get_link_status();

	mozart_module_mutex_lock();

	pr_debug("player_state = %d, link = %d\n", self->player_state, link);

	if (link == BT_LINK_CONNECTED) {
		if (self->player_state == player_state_idle ||
		    self->player_state == player_state_pause) {
			/* resume */
			if (__mozart_module_is_run(self)) {
				bt_avk_resume_handler();
				mozart_smartui_bt_toggle(true);
				self->player_state = player_state_play;
			} else if (__mozart_module_is_start(self)) {
				mozart_smartui_bt_toggle(true);
				self->player_state = player_state_play;
			}
		} else if (self->player_state == player_state_play) {
			/* pause */
			if (__mozart_module_is_run(self)) {
				bt_avk_pause_handler();
				mozart_smartui_bt_toggle(false);
				self->player_state = player_state_pause;
			} else if (__mozart_module_is_start(self)) {
				mozart_smartui_bt_toggle(false);
				self->player_state = player_state_pause;
			}
		}
	} else {
		mozart_bluetooth_auto_reconnect(USE_HS_AVK, 0);
	}

	mozart_module_mutex_unlock();
}

static void bt_avk_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (mozart_linein_is_in())
		mozart_linein_start(true);
	else if (__mozart_module_is_online()){
		#if (SUPPORT_VR == VR_ATALK)
			mozart_atalk_cloudplayer_start(true);
		#elif (SUPPORT_VR == VR_SPEECH)
			mozart_aitalk_cloudplayer_start(true);
		#endif
	}
	else{
		#if (SUPPORT_VR == VR_ATALK)
			mozart_atalk_localplayer_start(true);
		#elif (SUPPORT_VR == VR_SPEECH)
			mozart_aitalk_localplayer_start(true);
		#endif
	}
	mozart_module_mutex_unlock();
}

static void bt_avk_module_disconnect_handler(struct mozart_module_struct *self)
{
	self->player_state = player_state_idle;
	mozart_bluetooth_avk_stop_play();
	mozart_bluetooth_disconnect(USE_HS_AVK);
}

static struct mozart_module_struct bt_avk_module = {
	.name = "bt_avk",
	.priority = 1,
	.attach = module_unattach,
	.mops = {
		.on_start   = bt_avk_module_start,
		.on_run     = bt_avk_module_run,
		.on_suspend = bt_avk_module_suspend,
		.on_stop    = bt_avk_module_stop,
	},
	.kops = {
		.previous_song = bt_avk_module_previous_song,
		.volume_up = bt_avk_module_volume_up,
		.volume_down = bt_avk_module_volume_down,
		.next_song = bt_avk_module_next_song,
		.next_module = bt_avk_module_next_module,
		.resume_pause = bt_avk_module_resume_pause,
		.disconnect_handler = bt_avk_module_disconnect_handler,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_bt_avk_is_start(void)
{
	return __mozart_module_is_start(&bt_avk_module);
}

int mozart_bt_avk_start(bool in_lock)
{
	if (bt_avk_module.start) {
		return bt_avk_module.start(&bt_avk_module, module_cmd_stop, in_lock);
	} else {
		pr_err("bt_avk_module isn't registered!\n");
		return -1;
	}
}

int mozart_bt_avk_do_play(void)
{
	int i, ret, vol;
	module_status domain_status;
	struct mozart_module_struct *self = &bt_avk_module;

	/* wait 0.5s */
	for (i = 0; i < 10; i++) {
		if (share_mem_get(BT_AVK_DOMAIN, &domain_status)) {
			pr_err("share_mem_get failed!\n");
			return -1;
		}

		if (domain_status != WAIT_RESPONSE)
			usleep(50 * 1000);
		else
			break;
	}

	mozart_module_mutex_lock();
	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_play;
		mozart_smartui_bt_play();
		share_mem_set(BT_AVK_DOMAIN, RESPONSE_DONE);
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_play;
		mozart_smartui_bt_play();
		share_mem_set(BT_AVK_DOMAIN, RESPONSE_PAUSE);
	} else {
		ret = -1;
		share_mem_set(BT_AVK_DOMAIN, RESPONSE_CANCEL);
	}
	mozart_module_mutex_unlock();

	pr_debug("ret = %d\n", ret);

	vol = mozart_volume_get();
	mozart_bluetooth_avk_set_volume_up(bt_avk_volume_dsp2phone(&vol));

	return ret;
}

int mozart_bt_avk_do_play_without_shm(void)
{
	int ret, vol;
	struct mozart_module_struct *self = &bt_avk_module;

	mozart_module_mutex_lock();
	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_play;
		mozart_smartui_bt_play();
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_play;
		mozart_smartui_bt_play();
	} else {
		ret = -1;
		share_mem_set(BT_AVK_DOMAIN, RESPONSE_CANCEL);
	}
	mozart_module_mutex_unlock();

	pr_debug("ret = %d\n", ret);

	vol = mozart_volume_get();
	mozart_bluetooth_avk_set_volume_up(bt_avk_volume_dsp2phone(&vol));

	return ret;
}

int mozart_bt_avk_do_pause(void)
{
	int ret;
	struct mozart_module_struct *self = &bt_avk_module;

	mozart_smartui_bt_toggle(false);

	mozart_module_mutex_lock();
	if (self->state == module_state_run) {
		ret = 1;
		self->player_state = player_state_pause;
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
	} else {
		ret = -1;
	}
	mozart_module_mutex_unlock();

	pr_debug("ret = %d\n", ret);

	return ret;
}

int mozart_bt_avk_startup(void)
{
	if (mozart_module_register(&bt_avk_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
	//mozart_bt_avk_init_fft( );
	//start_bt();

	return 0;
}

int mozart_bt_avk_shutdown(void)
{
	mozart_bt_avk_uninit_fft( );
	if (bt_avk_module.stop)
		bt_avk_module.stop(&bt_avk_module, module_cmd_stop, false);
	mozart_module_unregister(&bt_avk_module);
	stop_bt();

	return 0;
}
