#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "vr-speech_interface.h"
#include "ini_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"

#include "vr-speech_interface.h"
#include "aiengine_app.h"
#include "mozart_aitalk_asr.h"
#include "mozart_aitalk_cloudplayer_control.h"

#include "mozart_config.h"

#ifndef MOZART_RELEASE
#define MOZART_AITALK_DEBUG
#endif

#ifdef MOZART_AITALK_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[SPEECH] %s: "fmt, __func__, ##args)
#else  /* MOZART_AITALK_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_AITALK_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[SPEECH] [Error] %s: "fmt, __func__, ##args)

/*******************************************************************************
 * module
 *******************************************************************************/
static void speech_asr_set_volume(bool set)
{
#ifdef FIXED_VOLUME
	int volume = 30;
	char buf[32] = {};

	if (set) {
		volume = 50;
		mozart_volume_set(volume, TONE_VOLUME);
	} else {
		if (mozart_ini_getkey("/usr/data/system.ini", "volume", "music", buf))
			printf("failed to parse /usr/data/system.ini, set music volume to 30.\n");
		else
			volume = atoi(buf);

		mozart_volume_set(volume, MUSIC_VOLUME);
	}
#endif
}

static int aitalk_asr_module_start(struct mozart_module_struct *current)
{
	pr_debug("------------------->>>>>> aitalk_asr_module_start\n");
	ai_sem_start();
	return 0;
}

static int aitalk_asr_module_run(struct mozart_module_struct *current)
{

	pr_debug("------------------->>>>>> aitalk_asr_module_run\n");
	speech_asr_set_volume(true);
	return 0;
}

static int aitalk_asr_module_suspend(struct mozart_module_struct *current)
{
	pr_debug("------------------->>>>>> aitalk_asr_module_suspend\n");
	speech_asr_set_volume(false);
	return 0;
}

static int aitalk_asr_module_stop(struct mozart_module_struct *current)
{
	pr_debug("------------------->>>>>> aitalk_asr_module_stop\n");
	speech_asr_set_volume(false);
	return 0;
}

#ifdef FIXED_VOLUME
static void aitalk_asr_module_volume_up(struct mozart_module_struct *self)
{
}

static void aitalk_asr_module_volume_down(struct mozart_module_struct *self)
{
}
#endif

static void aitalk_asr_module_wifi_config(struct mozart_module_struct *current)
{
	/* Don't support */
}

static void aitalk_asr_module_asr_cancel(struct mozart_module_struct *current)
{
}

static struct mozart_module_struct aitalk_asr_module = {
	.name = "speech_asr",
	.priority = 9,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = aitalk_asr_module_start,
		.on_run     = aitalk_asr_module_run,
		.on_suspend = aitalk_asr_module_suspend,
		.on_stop    = aitalk_asr_module_stop,
	},
	.kops = {
#ifdef FIXED_VOLUME
		.volume_up = aitalk_asr_module_volume_up,
		.volume_down = aitalk_asr_module_volume_down,
#endif
		.wifi_config = aitalk_asr_module_wifi_config,
		.asr_cancel = aitalk_asr_module_asr_cancel,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
int mozart_aitalk_asr_start(void)
{
	if (aitalk_asr_module.start) {
		return aitalk_asr_module.start(&aitalk_asr_module, module_cmd_suspend, false);
	} else {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
}


void mozart_ai_error(int error_type){
	switch (error_type){
	case  AI_ERROR_SEM_FAIL_1:
		mozart_prompt_tone_key_sync("error_sem_fail_1", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_start();
		break;
	case AI_ERROR_SEM_FAIL_2:
		mozart_prompt_tone_key_sync("error_sem_fail_2", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_start();
		break;
	case AI_ERROR_SEM_FAIL_3:
		mozart_prompt_tone_key_sync("error_sem_fail_3", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_ATUHORITY:
		mozart_prompt_tone_key_sync("error_authority", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_INVALID_DOMAIN:
		mozart_prompt_tone_key_sync("error_invalid_domain", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_SYSTEM:
		mozart_prompt_tone_key_sync("error_system", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_NO_VOICE:
		mozart_prompt_tone_key_sync("error_no_voice", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_SERVER_BUSY:
		mozart_prompt_tone_key_sync("error_server_busy", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_NET_SLOW:
		mozart_prompt_tone_key_sync("error_net_slow", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	case  AI_ERROR_NET_FAIL:
		mozart_prompt_tone_key_sync("error_net_fail", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	default:
		mozart_prompt_tone_key_sync("error_net_slow", false);
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
		break;
	}
}

#if 0
static int mozart_vr_speech_interface_callback(vr_info *recog_info)
{

	char *result;
	char *recog_asr_result = NULL;
	char *asr_result = NULL;

	printf("#### {%s, %s, %d} flag = %d ####\n", __FILE__, __func__, __LINE__, recog_info->recog_flag);
	if (recog_info->json_len != 0 && (recog_info->recog_asr_result + recog_info->json_len) != NULL) {
		recog_asr_result = recog_info->recog_asr_result;
		asr_result = recog_info->recog_asr_result + strlen(recog_info->recog_asr_result) + 1;
		printf("recog_asr_result: %s, asr_result: %s.\n", recog_asr_result, asr_result);
	}

	if (recog_info->recog_flag == WAKE_UP) {
		mozart_module_asr_wakeup();
	} else if (recog_info->recog_flag == SEARCH_MUSIC) {
		result = malloc(strlen(asr_result) + strlen("音乐:") + 1);
		sprintf(result, "%s%s", "音乐:", asr_result);
		mozart_smartui_asr_success(result);
		free(result);
		mozart_aitalk_asr_over();
	} else if (recog_info->recog_flag == NATIVE_ASR_CMD) {
		result = malloc(strlen(asr_result) + strlen("命令:") + 1);
		sprintf(result, "%s%s", "命令:", asr_result);
		mozart_smartui_asr_success(result);
		free(result);
		mozart_aitalk_asr_over();
	} else if(recog_info->recog_flag == TTS_ANSWER) {
		result = malloc(strlen(asr_result) + strlen("回答:") + 1);
		sprintf(result, "%s%s", "回答:", asr_result);
		mozart_smartui_asr_success(result);
		free(result);
		mozart_aitalk_asr_over();
	} else if(recog_info->recog_flag == WRONG) {
		mozart_smartui_asr_fail("语音识别失败");
		mozart_aitalk_asr_over();
	}

	return 0;//CLOUD;
}
#endif

static void *stop_func(void *args)
{
	pthread_detach(pthread_self());
	usleep(100*1000);
	mozart_smartui_asr_over();
	if (aitalk_asr_module.stop)
		aitalk_asr_module.stop(&aitalk_asr_module, module_cmd_run, false);
	else
		pr_err("mozart_module_register fail\n");

	return NULL;
}

int mozart_aitalk_asr_over(void)
{
	pthread_t stop_pthread;

	if (pthread_create(&stop_pthread, NULL, stop_func, NULL) == -1) {
		pr_err("create display delay fail\n");
		return -1;
	}

	return 0;
}

int mozart_aitalk_asr_startup(void)
{
	if (mozart_module_register(&aitalk_asr_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	ai_aitalk_send_init();
	//-----------------------startup aec
	ai_speech_startup(0, mozart_vr_speech_interface_callback);
	aitalk_cloudplayer_startup();
	return 0;
}

int mozart_aitalk_asr_shutdown(void)
{
	mozart_module_unregister(&aitalk_asr_module);
	//-----------------------shutdown aec
	ai_speech_shutdown();
	ai_aitalk_send_destroy();
	return 0;
}


