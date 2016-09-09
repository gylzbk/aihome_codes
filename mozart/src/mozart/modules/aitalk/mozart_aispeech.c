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
#include "mozart_aispeech.h"
#include "mozart_config.h"

#ifndef MOZART_RELEASE
#define MOZART_SPEECH_DEBUG
#endif

#ifdef MOZART_SPEECH_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[SPEECH] %s: "fmt, __func__, ##args)
#else  /* MOZART_SPEECH_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_SPEECH_DEBUG */

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

static int speech_asr_module_start(struct mozart_module_struct *current)
{
	return 0;
}

static int speech_asr_module_run(struct mozart_module_struct *current)
{
	speech_asr_set_volume(true);
	return 0;
}

static int speech_asr_module_suspend(struct mozart_module_struct *current)
{
	speech_asr_set_volume(false);
	return 0;
}

static int speech_asr_module_stop(struct mozart_module_struct *current)
{
	speech_asr_set_volume(false);
	return 0;
}

#ifdef FIXED_VOLUME
static void speech_asr_module_volume_up(struct mozart_module_struct *self)
{
}

static void speech_asr_module_volume_down(struct mozart_module_struct *self)
{
}
#endif

static void speech_asr_module_wifi_config(struct mozart_module_struct *current)
{
	/* Don't support */
}

static void speech_asr_module_asr_cancel(struct mozart_module_struct *current)
{
}

static struct mozart_module_struct speech_asr_module = {
	.name = "speech_asr",
	.priority = 9,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = speech_asr_module_start,
		.on_run     = speech_asr_module_run,
		.on_suspend = speech_asr_module_suspend,
		.on_stop    = speech_asr_module_stop,
	},
	.kops = {
#ifdef FIXED_VOLUME
		.volume_up = speech_asr_module_volume_up,
		.volume_down = speech_asr_module_volume_down,
#endif
		.wifi_config = speech_asr_module_wifi_config,
		.asr_cancel = speech_asr_module_asr_cancel,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
int mozart_speech_asr_start(void)
{
	if (speech_asr_module.start) {
		return speech_asr_module.start(&speech_asr_module, module_cmd_suspend, false);
	} else {
		pr_err("mozart_module_register fail\n");
		return -1;
	}
}

static __attribute__((unused)) int mozart_vr_speech_interface_callback(vr_info *recog_info)
{
#if 0
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
		mozart_speech_asr_over();
	} else if (recog_info->recog_flag == NATIVE_ASR_CMD) {
		result = malloc(strlen(asr_result) + strlen("命令:") + 1);
		sprintf(result, "%s%s", "命令:", asr_result);
		mozart_smartui_asr_success(result);
		free(result);
		mozart_speech_asr_over();
	} else if(recog_info->recog_flag == TTS_ANSWER) {
		result = malloc(strlen(asr_result) + strlen("回答:") + 1);
		sprintf(result, "%s%s", "回答:", asr_result);
		mozart_smartui_asr_success(result);
		free(result);
		mozart_speech_asr_over();
	} else if(recog_info->recog_flag == WRONG) {
		mozart_smartui_asr_fail("语音识别失败");
		mozart_speech_asr_over();
	}
#endif
	return 0;//CLOUD;
}

static void *stop_func(void *args)
{
	usleep(100*1000);
	mozart_smartui_asr_over();
	if (speech_asr_module.stop)
		speech_asr_module.stop(&speech_asr_module, module_cmd_run, false);
	else
		pr_err("mozart_module_register fail\n");

	return NULL;
}

int mozart_speech_asr_over(void)
{
#if (SUPPORT_VR == VR_SPEECH)
		ai_set_enable(true);
#endif
	pthread_t stop_pthread;

	if (pthread_create(&stop_pthread, NULL, stop_func, NULL) == -1) {
		pr_err("create display delay fail\n");
		return -1;
	}
	pthread_detach(stop_pthread);
	return 0;
}

int mozart_speech_startup(int wakeup_mode_mark)
{
	//-----------------------startup asr
	if (mozart_module_register(&speech_asr_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	//-----------------------startup aec
	if (ai_speech_get_status() != STATUS_NULL){
		ai_speech_shutdown();
	}
	ai_speech_startup(wakeup_mode_mark, mozart_vr_speech_interface_callback);

	return 0;
}

int mozart_speech_shutdown(void)
{
	//-----------------------shutdown aec
	ai_speech_shutdown();
	//-----------------------shutdown asr
	mozart_speech_asr_over();
	mozart_module_unregister(&speech_asr_module);

	return 0;
}
