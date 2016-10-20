#ifndef __AI_SLOT_H_
#define __AI_SLOT_H_

#include "aiengine.h"
#include "vr-speech_interface.h"
#include "cJSON.h"

/**
 * @ =========================================================================
 *   音乐 识别返回结果  定义.
 */
typedef struct ai_sem_search_t{
	char *search_words;
	char *request;
	char *url;
}ai_sem_search_t;

typedef struct ai_param_data_t {
	cJSON *param_j;		//	参数结果 - json
	char  *param_s;		//	参数名称 - string
	char  *value_s;		//	参数值 - string
}ai_param_data_t;

typedef struct ai_sem_result_t {
	ai_param_data_t  *input;	//
	ai_param_data_t  *param;	//
}ai_sem_result_t;


#define AI_SLOT_MUSIC_MAX 2
typedef struct ai_slot_music_t {
//------------------ 音乐
	char *singer;	//	歌手名	:	周杰伦
	char *song;		//	歌曲名	:	夜曲
	char *album;	//	专辑名	:	第七章
	char *style;	//	风格		:	忧伤,欢快
	char *type;		//	类型		:	 军歌,流行歌曲,儿歌
	char *list;		//	播放列表	*/
	char *source;
}ai_slot_music_t;
extern const char *aiSlotMusicName[AI_SLOT_MUSIC_MAX];

#define AI_SLOT_RADIO_MAX 6
typedef struct ai_slot_radio_t{
	char *keyword;	//	关键字:
	char *family;	//	类别	:	小品
	char *column;	//	栏目	:	笑话大全
	char *program;	//	节目	:	播放小品卖拐
	char *artist;	//	艺人	:	赵本山
	char *serial_number;		//	序列号:	第一期
}ai_slot_radio_t;	//*/
extern const char *aiSlotRadioName[AI_SLOT_RADIO_MAX];


#define AI_SLOT_WEATHER_MAX 4
typedef struct ai_slot_weather_t {
	char *city;		//	城市	:	深圳市
	char *county;	//	区县	:	南山区
	char *day;		//	日期	:	今天，明天
	char *time;		//	时间	:	八点，下午
}ai_slot_weather_t;
extern const char *aiSlotWeatherName[AI_SLOT_WEATHER_MAX];
/**
 * @ 中控领域 slot 定义.
 */

#define AI_SLOT_CONTROL_MAX 1
#define AI_SLOT_CONTROL_VALUE_MAX 1
typedef struct ai_slot_coltrol_t {
	char *volume;		//	音量	:
/*	char *brightness;	//	亮度	:	一/二   打开空调一
	char *color;		//	颜色	:	今天，明天
	char *timer;		//	定时	:	黄色/白色
	char *operation;	//	操作	:	//*/
}ai_slot_coltrol_t;


/**
 * @ 领域 slot 定义.
 */
typedef struct ai_debug_time_s {
	long t_sem;			// time of sem
	long t_tts;			// time of tts
	int t_tts_play;		// time of tts to play
	int is_tts_play;
	long t_all;			// time of all
}ai_debug_time_s;

extern 	ai_debug_time_s ai_time;
/**
 * @ 领域 slot 定义.
 */
typedef struct ai_slot_t {
	ai_slot_music_t music;
	ai_slot_radio_t radio;
	ai_slot_weather_t weather;
//	ai_slot_coltrol_t control;
}ai_slot_t;

extern ai_sem_search_t ai_sem_search;
extern ai_slot_t ai_slot;

extern int ai_slot_resolve(vr_info *recog,cJSON *sem_json);
//extern int ai_slot_resolve(vr_info *recog,char *sem_param);
extern const char *aiSlotRadioName[AI_SLOT_RADIO_MAX];


#endif
