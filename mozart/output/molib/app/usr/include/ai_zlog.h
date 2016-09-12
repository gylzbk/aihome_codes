/*********************************************************************
 * Copyright(C), 2016, aispeech CO., LTD.
 * Filename: mozart_log.h
 * Author: zhenquan.qiu
 * Version: V1.0.0
 * Date: 06/04 2016
 * Description:
 * Others:
 * History:
 *********************************************************************/
#ifndef __AI_ZLOG_H__
#define __AI_ZLOG_H__
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef AI_ZLOG_GB
#define AI_ZLOG_H_EX
#else
#define AI_ZLOG_H_EX extern
#endif

typedef enum {
	LOG_FATAL,
	LOG_ERROR,

	LOG_FACTORY_SETTING,
	LOG_KEY,
	LOG_VOLUME,
	LOG_CHANNEL,
	LOG_POWER,
	LOG_VR,
	LOG_DEBUG,


	LOG_MAX
}log_type_e;

AI_ZLOG_H_EX int ai_log_init(void);
AI_ZLOG_H_EX void ai_log_set_enable(bool enalbe);
AI_ZLOG_H_EX int ai_log_add(log_type_e type, const char *fmt, ...);
AI_ZLOG_H_EX void ai_log_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* end __AI_ZLOG_H_H__ */

