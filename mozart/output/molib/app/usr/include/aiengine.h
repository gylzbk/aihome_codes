#ifndef AIENGINE_H_
#define AIENGINE_H_
#include <sys/time.h>
#define AIENGINE_VERSION "2.7.8"

#if (!(defined AIENGINE_CALL) || !(defined AIENGINE_IMPORT_OR_EXPORT))
#    if defined __WIN32__ || defined _WIN32 || defined _WIN64
#       define AIENGINE_CALL __stdcall
#       ifdef  AIENGINE_IMPLEMENTION
#           define AIENGINE_IMPORT_OR_EXPORT __declspec(dllexport)
#       else
#           define AIENGINE_IMPORT_OR_EXPORT __declspec(dllimport)
#       endif
#    elif defined __ANDROID__
#       define AIENGINE_CALL
#       define AIENGINE_IMPORT_OR_EXPORT
#    else
#       define AIENGINE_CALL
#       define AIENGINE_IMPORT_OR_EXPORT __attribute ((visibility("default")))
#    endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct   timeval t_debug;

//#define __DEBUG_TIME__
#define __DEBUG__
#ifdef __DEBUG__
#ifdef __DEBUG_TIME__
#define DEBUG(format, ...) {gettimeofday(&t_debug, NULL); printf("[%ld.%ld][%s : %s : %d] ",t_debug.tv_sec,t_debug.tv_usec,__FILE__,__func__,__LINE__); printf(format, ##__VA_ARGS__);}
#define FDEBUG(format, ...) fprintf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) {printf("[%s : %s : %d] ",__FILE__,__func__,__LINE__); printf(format, ##__VA_ARGS__);}
#define FDEBUG(format, ...) fprintf(format, ##__VA_ARGS__)
#endif
#else
#define DEBUG(format, ...)
#define FDEBUG(format, ...)
#endif

//#define __ERROR_ZLOG__
#ifdef __ERROR_ZLOG__
#include "ai_zlog.h"
#define PERROR(format, ...) {gettimeofday(&t_debug, NULL); printf("[%ld.%ld][%s : %s : %d] ",t_debug.tv_sec,t_debug.tv_usec,__FILE__,__func__,__LINE__); printf(format, ##__VA_ARGS__);\
								ai_log_add(LOG_ERROR,format, ##__VA_ARGS__);}
#else
#define PERROR(format, ...) {gettimeofday(&t_debug, NULL); printf("[%ld.%ld][%s : %s : %d] ",t_debug.tv_sec,t_debug.tv_usec,__FILE__,__func__,__LINE__); printf(format, ##__VA_ARGS__);}
#endif

#include "cJSON.h"
enum {
    AIENGINE_MESSAGE_TYPE_JSON = 1,
    AIENGINE_MESSAGE_TYPE_BIN
};

struct aiengine;

typedef int (AIENGINE_CALL *aiengine_callback)(void *usrdata, const char *id, int type, const void *message, int size);
AIENGINE_IMPORT_OR_EXPORT struct aiengine * AIENGINE_CALL aiengine_new(const char *cfg);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_delete(struct aiengine *engine);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_start(struct aiengine *engine, const char *param, char id[64], aiengine_callback callback, const void *usrdata);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_feed(struct aiengine *engine, const void *data, int size);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_stop(struct aiengine *engine);
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_cancel(struct aiengine *engine);
#ifdef USE_ECHO
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_echo(struct aiengine *engine, const void *rec, const void *play, int size, int flag);
#endif
/*
 *int opt : 1:获取aiengine版本号；2: 设置wifi状态；3: 设置设备ID;
 *          4:获取aiengine生成的recordId; 5: 发送JSON结构的日志； 6: 获取设备ID;
 *          7:获取vad数据 8: 9:资源更新; 10:查询授权状态; 11: 授权
 * */
AIENGINE_IMPORT_OR_EXPORT int AIENGINE_CALL aiengine_opt(struct aiengine *engine, int opt, char *data, int size);

#ifdef __cplusplus
}
#endif
#endif

