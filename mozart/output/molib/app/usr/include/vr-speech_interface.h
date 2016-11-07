#ifndef __SPEECH_INTERFACE_H_
#define __SPEECH_INTERFACE_H_
#include "music_list.h"
//#define SPEECH_CLOUD_TTS
#include <stdbool.h>

typedef enum aiengine_status{
	AIENGINE_STATUS_INIT = 0,				//
//	AIENGINE_STATUS_IDEL,					//
	AIENGINE_STATUS_AEC,					//
	AIENGINE_STATUS_SEM,					//
	AIENGINE_STATUS_PROCESS,				//
	AIENGINE_STATUS_ERROR,					//
	AIENGINE_STATUS_STOP,					//
	AIENGINE_STATUS_EXIT,					//
//	AIENGINE_STATUS_ASRING,					//
//	AIENGINE_STATUS_UNNET,					//
//	AIENGINE_STATUS_AEC,				//
//	AIENGINE_STATUS_WAIT_WAKEUP,			//
//	AIENGINE_STATUS_SEM,				//
//	AIENGINE_STATUS_ASR_FAIL,				//
//	AIENGINE_STATUS_ASR_SUCCESS,			//
//	AIENGINE_STATUS_ASR_EXIT,			//
//	AIENGINE_STATUS_TTS_ANSWER,				//
//	AIENGINE_STATUS_WAIT_TO_PLAY,			//
//	AIENGINE_STATUS_STOP_AEC,				//	stop wakeup engine
//	AIENGINE_STATUS_STOP_CLOUDSEM,				//	stop asr engine
//	AIENGINE_STATUS_RESEART,					//
	AIENGINE_STATUS_MAX
}aiengine_status;

typedef enum vr_speech_status_type{
	VR_SPEECH_NULL = 0,
	VR_SPEECH_INIT,
	VR_SPEECH_QUIT,
	VR_SPEECH_ERROR
}vr_speech_status_type;

typedef enum aec_status_t{
	AEC_STATUS_INVALID =0,
	AEC_STATUS_INIT,				//	1
	AEC_STATUS_IDEL,				//	2
	AEC_STATUS_START,				//	3
	AEC_STATUS_WAIT_WAKEUP,			//	4
	AEC_STATUS_WAKEUP,				//	5
	AEC_STATUS_STOP,				//	6
	AEC_STATUS_ERROR,				//	7
	AEC_STATUS_EXIT,				//	8
	AEC_STATUS_MAX
}aec_status_t;

typedef enum cloudsem_status_t{
	CLOUDSEM_STATUS_INVALID =0,
	CLOUDSEM_STATUS_INIT,				// 1
	CLOUDSEM_STATUS_IDEL,				//	2
	CLOUDSEM_STATUS_START,				//	3
	CLOUDSEM_STATUS_WAIT_SPEED,			//	4
	CLOUDSEM_STATUS_ASR_SUCCESS,	//	5
	CLOUDSEM_STATUS_ASR_FAIL,			//	6
	CLOUDSEM_STATUS_STOP,				//	7
	CLOUDSEM_STATUS_ERROR,				//	8
	CLOUDSEM_STATUS_EXIT,				//	9
	CLOUDSEM_STATUS_MAX
}cloudsem_status_t;
//*/
typedef enum recog_intent{
	RECOG_INTENT_NULL = 0,
	RECOG_INTENT_COMMAND,					//	单一指令
	RECOG_INTENT_SEARTH_MUSIC,				//	搜索歌曲
	RECOG_INTENT_TTS_ANSWER,				//	返回TTS
}recog_intent;


//*/
typedef enum recog_server_e{
	RECOG_SERVER_TO_CONTINUE = 0,
	RECOG_SERVER_TO_WAKEUP,
	RECOG_SERVER_TO_END,
	RECOG_SERVER_SEM_ERROR,
	RECOG_SERVER_SEM_ERROR_END,
	RECOG_SERVER_OTHER_ERROR,
}recog_server_e;

typedef enum recog_function {
	RECOG_FUN_NULL = 0,
	RECOG_FUN_PLAY,			//	play
	RECOG_FUN_STOP,			//	stop
	RECOG_FUN_PAUSE,		//	pause
	RECOG_FUN_PREVIOUS_SONG,	//	previous_song,
	RECOG_FUN_NEXT_SONG,	//	next_song
	RECOG_FUN_VOLUME,		//	volume
	RECOG_FUN_LIST_DEL,		//	清空列表
	RECOG_FUN_LIST_READ,	//	列表状态
	RECOG_FUN_NET_STATUS,	//	net_status,
	RECOG_FUN_WIFI_MODE,	//	wifi_mode
	RECOG_FUN_CONFIG_NET,	//	config_net
	RECOG_FUN_BATTERY,		//	battery
}recog_function;


typedef enum recog_domain{
	RECOG_DOMAIN_NULL,
	RECOG_DOMAIN_CHAT,
	RECOG_DOMAIN_MUSIC,
	RECOG_DOMAIN_WEATHER,
	RECOG_DOMAIN_CALENDAR,
	RECOG_DOMAIN_COMMAND,
	RECOG_DOMAIN_NETFM,
	RECOG_DOMAIN_REMINDER,
	RECOG_DOMAIN_STOCK,
	RECOG_DOMAIN_POETRY,
	RECOG_DOMAIN_MOVIE,
	RECOG_DOMAIN_MAX
}recog_domain;	//*/

typedef enum sds_state_e{
	SDS_STATE_NULL,
	SDS_STATE_DO,
	SDS_STATE_QUERY,				//	query
	SDS_STATE_OFFERNONE,			//	offernone
	SDS_STATE_OFFER,				//	offer
	SDS_STATE_INTERACT,				//	interact
	SDS_STATE_EXIT,					//	exit
	SDS_STATE_MAX
}sds_state_e;	//*/

typedef enum sds_object_e{
	SDS_OBJECT_NULL,
	SDS_OBJECT_VOLUME,
	SDS_OBJECT_MUSIC,
	SDS_OBJECT_MAX
}sds_object_e;	//*/

typedef enum sds_command_type_e{
	SDS_COMMAND_NULL,
	SDS_COMMAND_VOLUME,
	SDS_COMMAND_MUSIC_PLAY,
	SDS_COMMAND_MUSIC_PAUSE,
	SDS_COMMAND_MUSIC_RESUME,
	SDS_COMMAND_MUSIC_STOP,
	SDS_COMMAND_MUSIC_PREVIOUS,
	SDS_COMMAND_MUSIC_NEXT,
	SDS_COMMAND_EXIT,
	SDS_COMMAND_ELIFE,
	SDS_COMMAND_MAX,
}sds_command_type_e;	//*/


#if 0
typedef struct music_info_t {
	char *title;
	char *artist;
	char *url;
}music_info;
#endif

typedef struct movie_info_t {
	char *name;
	char *sequence;
	char *director;
	char *player;
	char *type ;
	char *area ;
}movie_info;


typedef struct weather_info_t {
	char *weather;
	char *temperature;
	char *wind;
	char *area;
}weather_info;

typedef struct vr_info {
	int status;
	int next_status;
	int status_last;
	bool key_record_stop;
	bool is_control_play_music;
//	int intent;
	int domain;
//	int tts_enable;
//	int function;
	int state;
	int music_num;
//	int command_type;
//	char *command_value;
	int error_id;
	int error_type;
	char *recordId;
	char *env;
	char *semantics;
	char* operation;
	char* scene;
	char* device;
	char* location;
	char* volume;
	char *object;
	char *input;
	char *output;
	char *contextId;
	int sds_flag;
	char *search_artist;
	char *search_title;
	music_info music;
	weather_info weather;
	movie_info movie;
//	bool is_get_song_recommend;
//	bool is_play_song_recommend;
    char *date;
    char *time;
	char *event;
}vr_info;


typedef enum recog_flag_status {
	ASR = 0,
	SEARCH_MUSIC,
	NATIVE_ASR_CMD,
	TTS_ANSWER,
	WAKE_UP,
	WRONG,
}recog_flag_status;

/**
 * @brief voice wakeup mode.
 */
typedef enum wakeup_mode {
	AI_VOICE_WAKEUP = 0,
	AI_KEY_SHORTPRESS_WAKEUP,
	AI_KEY_LONGPRESS_WAKEUP,
}wakeup_mode;

/**
 * @brief cloud or native mode.
 */
typedef enum asr_mode {
	AI_LICENSE = -1,
	AI_CLOUD = 0,
	AI_OFFLINE = 1,
}asr_mode;

/**
 * @brief voice recognition result struct.
 */

extern int ai_set_enable(bool enable);

extern int ai_aec_set_enable(bool enable);

typedef int (*mozart_vr_speech_callback)(vr_info * recog_info);

/**
 * @brief checkout statue voice recognition of speech
 */
extern int ai_speech_get_status(void);

/**
 * @brief init voice recognition of speech
 */
extern int ai_speech_startup(int wakeup_mode, mozart_vr_speech_callback callback);

/**
 * @brief quit voice recognition of speech
 */
extern int ai_speech_shutdown(void);

/**
 * @brief wakeup voice recognition of speech
 */
extern int mozart_key_wakeup(void);

/**
 * @brief key control record end voice recognition of speech
 */

//extern int mozart_key_asr_record_end(void);

//extern int mozart_tts(char *TTS_text);

#endif
