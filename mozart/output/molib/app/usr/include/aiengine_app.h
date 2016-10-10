#ifndef _AIENGINE_APP_H_
#define _AIENGINE_APP_H_
#include <sys/types.h>
#include <stdbool.h>
#include "record_interface.h"
#include "tips_interface.h"
#include "ini_interface.h"
#include "sharememory_interface.h"
#include "cJSON.h"
#include "echo_wakeup.h"
#include "aiengine.h"
#include "ai_error.h"
#include "ai_server.h"
#include "vr-speech_interface.h"
#include <semaphore.h>

#include "ai_aitalk_send.h"


#define CLOUD_TTS_MP3 "/tmp/speech_asr_result.mp3"
#define AI_CONTROL_MOZART 1

#define AI_CONTROL_MOZART_ATALK 1

#include "vr-speech_interface.h"

extern int asr_mode_cfg;
extern int fd_dsp_rd;

enum AEC_STATUS_TYEP
{
    AEC_IDEL = 0,
	AEC_START,
    AEC_WAKEUP,
    AEC_WAKEUP_TID2_EXIT,
    AEC_WAKEUP_TID1_EXIT,
    AEC_END,
}AEC_STATUS_TYEP;

typedef enum SEM_STATUS_TYPE
{
    SEM_STATUS_IDEL = 0,
	SEM_STATUS_START,
    SEM_STATUS_SUCCESS,
    SEM_STATUS_FAIL,
    SEM_STATUS_END
}SEM_STATUS_TYPE;

typedef enum SEM_RESULE_TYPE
{
    SEM_SUCCESS,
    SEM_FAIL,
    SEM_NET_LOW,
    SEM_EXIT,
}SEM_RESULE_TYPE;

extern struct mic_record *record_info;
extern unsigned long record_len;
extern void *record_buf;
extern vr_info recog;

typedef struct ai_status_s{
	vr_speech_status_type vr_status;
	bool is_init;
	bool aec_enable;
	bool is_running;
	bool is_working;
	bool run_enable;
	bool quit_finish;
	int mode_cfg;
	bool is_play_music;
	int asr_mode_cfg;
//	echo_wakeup_t *ew;
//	struct aiengine *agn;
//	pthread_mutex_t mutex_lock;
	vr_info recog;
	mozart_vr_speech_callback vr_callback_pointer;
}ai_status_s;

extern ai_status_s ai_flag;

typedef struct ai_sem_flag_t{
	bool error;
	bool speak_end;
	enum SEM_STATUS_TYPE state;
	enum SEM_RESULE_TYPE result;
	bool set_end;
}ai_sem_flag_t;
extern  struct ai_sem_flag_t ai_sem_flag;

typedef struct ai_aec_flag_t{
	bool error;
	enum AEC_STATUS_TYEP state;
	enum SEM_RESULE_TYPE result;
	bool set_end;
}ai_aec_flag_t;
extern  ai_aec_flag_t ai_aec_flag;

struct aiengine * aiengine_new(const char *cfg);
int aiengine_delete(struct aiengine *engine);
int aiengine_start(struct aiengine *engine, const char *param, char id[64], aiengine_callback callback, const void *usrdata);
int aiengine_feed(struct aiengine *engine, const void *data, int size);
int aiengine_stop(struct aiengine *engine);
//int aiengine_cancel(struct aiengine *engine, struct mic_record * record_info);
int aiengine_cancel(struct aiengine *engine);
int aiengine_echo(struct aiengine *engine, const void *rec, const void *play, int size, int flag);


#define RECORD_BUFSZ                (3200)
#define TMP_BUFFER_SZ               (1024)
//#define STR_BUFFER_SZ               (2048)


extern void * aec_init(void);
extern int ai_aec(echo_wakeup_t *ew);
extern int ai_cloud_tts(struct aiengine *agn, char *SynTxt);


extern const char *aiSlotDomain[RECOG_DOMAIN_MAX];
extern const char *aiSlotObject[SDS_OBJECT_MAX];
extern const char *aiSlotState[SDS_STATE_MAX];
extern const char *aiengineStatus[AIENGINE_STATUS_MAX];
extern const char *aiSlotCommand[SDS_COMMAND_MAX];

extern int   ai_cloud_sem_stoping(void);
extern void ai_cloud_sem_stop(void);
extern void ai_cloud_sem_free(void);

extern void ai_cloud_sem_text_stop(void);
extern int ai_cloud_sem_text(char *text);

extern void  ai_aec_stop(void);
extern int ai_aiengine_start(void);
extern int ai_aiengine_stop(void);
extern int ai_aiengine_delete(void);
extern int ai_aitalk_sem_init(void);
extern int ai_aitalk_sem_destory(void);
//#ifdef SYN_TOO_LONG
extern struct   timeval   t_sem_start,t_sem_end,t_tts_start,t_tts_end,t_play,t_all;
//extern int tim_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
extern int time_subtract(unsigned long *dff,struct timeval x, struct timeval y);
//#endif
extern int ai_recog_free(void);
extern int ai_tts_time(char *data);
extern int ai_tts_play_time(void);
extern int ai_tts(char *data,int enable_stop);
extern int time_subtract(unsigned long *dff,struct timeval x, struct timeval y);

extern int ai_speech_startup(int wakeup_mode, mozart_vr_speech_callback callback);
extern int ai_speech_shutdown(void);
extern int ai_set_enable(bool enable);
extern int ai_aiengine_stop(void);
extern int ai_aecing(void);
extern int ai_sem_start(void);
extern int ai_key_record_wakeup(void);
extern int ai_key_record_stop(void);

extern int ai_play_music_order(int order);
extern bool ai_is_play_music(void);

#ifdef __cplusplus
}
#endif
#endif
