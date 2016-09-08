#ifndef _AI_ERROR_H_
#define _AI_ERROR_H_
#include <sys/types.h>
#include <stdbool.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_PRINT 1

typedef enum ai_error_e{
	AI_ERROR_NONE,
	AI_ERROR_ATUHORITY,
	AI_ERROR_INVALID_DOMAIN,
	AI_ERROR_SYSTEM,
	AI_ERROR_NO_VOICE,
	AI_ERROR_SEM_FAIL_1,
	AI_ERROR_SEM_FAIL_2,
	AI_ERROR_SEM_FAIL_3,
	AI_ERROR_SERVER_BUSY,
	AI_ERROR_NET_SLOW,
	AI_ERROR_NET_FAIL,
	AI_ERROR_UNKNOW,
	AI_ERROR_MAX
}ai_error_e;	//*/

extern int ai_error_get_id(int error_id);
extern const char *error_type[AI_ERROR_MAX];

#ifdef __cplusplus
}
#endif
#endif
