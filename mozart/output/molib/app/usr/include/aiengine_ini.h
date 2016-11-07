#ifndef _AIENGINE_INI_H_
#define _AIENGINE_INI_H_
#include <sys/types.h>
#include <stdbool.h>


typedef struct ini_asr_s{
	int record_time;
	int wait_time;
}ini_asr_s;

typedef struct ini_sds_s{
	bool is_multi;
}ini_sds_s;


typedef struct ini_server_s{
	bool is_auto_play;
}ini_server_s;

typedef struct ini_aiengine_s{
	ini_asr_s asr;
	ini_sds_s sds;
	ini_server_s server;
}ini_aiengine_s;

extern void aiengine_init_get_config(ini_aiengine_s *ini);

#endif
