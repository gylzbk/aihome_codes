#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include "aiengine_app.h"
#include "aiengine_ini.h"

void aiengine_init_get_config(ini_aiengine_s *ini){
	char buf[32] = {0};
	int time = 0;
	int ret = 0;
	//------------------------------ record time
	ini->asr.record_time = 80;			//default 8s
	if (mozart_ini_getkey("/usr/data/aiengine.ini", "asr", "record_time", buf)){
		PERROR("failed to parse /usr/data/aiengine.ini, set default asr record time to 8 s.\n");
	}
	else{
		time = atoi(buf);
		if ((time <5 || time > 15)){
			PERROR("error set asr record time to %d s false(5-15). set default to 8 s\n",time);
  		} else {
			ini->asr.record_time = time * 10;	//	time * 100ms
			DEBUG("set asr record time to %d s.\n", time);
		}
	}
	//------------------------------ wait result time
	ini->asr.wait_time = 10000;			// default	10s
	if (mozart_ini_getkey("/usr/data/aiengine.ini", "asr", "wait_time", buf)){
		PERROR("failed to parse /usr/data/aiengine.ini, set default asr wait time to 10 s.\n");
	}
	else{
		time = atoi(buf);
		if ((time <5 || time > 20)){
			PERROR("error set asr wait time to %d s false(5-20). set default to 10 s\n",time);
  		} else {
			ini->asr.wait_time = time * 1000;
			DEBUG("set asr wait time to %d s.\n", time);
		}
	}


	//------------------------------ sds multi
	ini->sds.is_multi = true;			//
	if (mozart_ini_getkey("/usr/data/aiengine.ini", "sds", "sds_multi", buf)){
		PERROR("failed to parse /usr/data/aiengine.ini, set default sds is_multi = 1.\n");
	}
	else{
		ret = atoi(buf);
		if (ret){
			ini->sds.is_multi = true;			//
  		} else {
			ini->sds.is_multi = false;			//
		}
		DEBUG("set multi to %d .\n", ret);
	}//*/

	//------------------------------ server auto play
	ini->server.is_auto_play= true;			//
	if (mozart_ini_getkey("/usr/data/aiengine.ini", "server", "auto_play", buf)){
		PERROR("failed to parse /usr/data/aiengine.ini, set default is_auto_play = 1.\n");
	}
	else{
		ret = atoi(buf);
		if (ret){
			ini->server.is_auto_play = true;			//
  		} else {
			ini->server.is_auto_play = false;			//
		}
		DEBUG("set auto_play to %d .\n", ret);
	}//*/
}



