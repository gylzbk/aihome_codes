#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <linux/soundcard.h>
#include "vr-speech_interface.h"
#include "aiengine.h"
#include "aiengine_app.h"
#include "ai_slot.h"

#if AI_CONTROL_MOZART
#include "mozart_key.h"
#endif

#if AI_CONTROL_MOZART_ATALK
#include "mozart_module.h"
#endif

//#include "ai_aec_thr.h"
//#include "ai_cloudsem_thr.h"
#include "echo_wakeup.h"

#include <semaphore.h>
//char recog_buf[STR_BUFFER_SZ] = {0};
static const char *version =
"\n=============================\n"\
"AIHOME: Aitalk for DS1825-pro\n"\
"Ver : 1.1.0\n"\
"Date: 2016-09-05\n"\
"=============================\n";

//		\"server\": \"ws://s-test.api.aispeech.com:10000\"\
//		\"server\": \"ws://112.80.39.95:8009\"\

#if 1
static const char *ew_cfg =
"\
{\
    \"luaPath\": \"/usr/fs/usr/share/vr/bin/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"/usr/fs/usr/share/vr/bin/prov-jz-2.7.8-mac.file-20201031\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"/mnt/sdcard/a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"/usr/fs/usr/share/vr/bin/vad.aihome.0.3.1027.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"native\": {\
        \"cn.dnn\": {\
            \"resBinPath\": \"/usr/fs/usr/share/vr/bin/aihome.1030.bin\"\
        },\
        \"cn.echo\": {\
            \"frameLen\": 512,\
            \"filterLen\": 2048,\
            \"rate\": 16000\
        }\
    }\
}";

static const char *agn_cfg =
"\
{\
    \"luaPath\": \"/usr/fs/usr/share/vr/bin/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"/usr/fs/usr/share/vr/bin/prov-jz-2.7.8-mac.file-20201031\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"/mnt/sdcard/a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"/usr/fs/usr/share/vr/bin/vad.aihome.0.3.1027.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"cloud\": {\
		\"server\": \"ws://s-test.api.aispeech.com:10000\"\
    }\
}";

#else
static const char *cfg =
"\
{\
    \"luaPath\": \"./bin/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"./bin/prov-jz-2.7.8-mac.file-20201031\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"/mnt/sdcard/a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"./bin/vad.aihome.0.3.1027.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"cloud\": {\
		\"server\": \"ws://s-test.api.aispeech.com:10000\"\
    },\
    \"native\": {\
        \"cn.dnn\": {\
            \"resBinPath\": \"./bin/aihome.1030.bin\"\
        },\
        \"cn.echo\": {\
            \"frameLen\": 512,\
            \"filterLen\": 2048,\
            \"rate\": 16000\
        }\
    }\
}";
#endif

//pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

vr_speech_status_type vr_speech_status = VR_SPEECH_NULL;

vr_info recog;

int asr_mode_cfg = AI_CLOUD;
static int asr_quit_finish = 0;
mozart_vr_speech_callback vr_speech_callback_pointer;
bool is_aiengine_enable = false;
bool is_aiengine_init = false;

bool is_vr_speech_work_flag = false;
bool is_seming = false;

//struct aiengine_thr_s aec,sem;
struct timeval t_debug;
struct aiengine *agn = NULL;
echo_wakeup_t *ew = NULL;

bool aitalk_play_music = false;

#ifdef SYN_TOO_LONG
#include <sys/time.h>
#include "ai_zlog.h"
struct   timeval   t_sem_start,t_sem_end;
struct   timeval t_tts_start,t_tts_end;
struct   timeval t_start,t_play;
struct   timeval   t_tone_call,t_tone_start,  t_tone_end;

int time_subtract(unsigned long *dff,struct timeval x, struct timeval y)
{
	struct timeval result;
	unsigned long timer;
    if ( x.tv_sec > y.tv_sec )
    return   -1;
    if ((x.tv_sec==y.tv_sec) && (x.tv_usec>y.tv_usec))
    return   -1;
    result.tv_sec = ( y.tv_sec-x.tv_sec );
    result.tv_usec = ( y.tv_usec-x.tv_usec );
    if (result.tv_usec<0)
    {
        result.tv_sec--;
        result.tv_usec+=1000000;
    }

	timer =result.tv_sec;
	timer *= 1000000;
	timer += result.tv_usec;
	*dff = timer/1000;
    return   0;
}
/*
int ai_tts_play_time(void){
	unsigned   long time;
	gettimeofday(&t_play,NULL);
	time_subtract(&time,t_tts_end,t_play);
	printf("tts_play: %12d\n",time);
//	ai_time_reset();
	return 0;
}
	//*/
int ai_sem_time(char *data){
	if (data == NULL)
		return -1;

	unsigned   long time = 0;
	time_subtract(&time,t_sem_start,t_sem_end);
	printf("sem: %6d\n",time);
	if ((recog.recordId)){//&&(time > 1500)){
		DEBUG("LOG-SEM: : rID:%s, t:%6d, sem:%s\n",recog.recordId,time,data);//*/
		ai_log_add(LOG_DEBUG,"SEM: rID: %s , t: %6d , sem: %s",recog.recordId,time,data);//*/
	}
	return 0;
}

int ai_tone_time_start(void){
  	gettimeofday(&t_tone_start,NULL);
	return 0;
}

int ai_tone_time_end(void){
	unsigned   long time = 0;
  	gettimeofday(&t_tone_end,NULL);
	time_subtract(&time,t_tone_start,t_tone_end);
	DEBUG("ai_tone: %6d\n",time);
	return 0;
}


int ai_tts_time(char *data){
	if (data == NULL)
		return -1;

	unsigned   long time = 0;
	time_subtract(&time,t_tts_start,t_tts_end);
	printf("tts: %6d\n",time);
	if ((recog.recordId)){//&&(time > 1500)){
		DEBUG("LOG-TTS: rID:%s, t:%6d, tts:%s\n",recog.recordId,time,data);//*/
		ai_log_add(LOG_DEBUG,"TTS: rID: %s , t: %6d, tts: %s",recog.recordId,time,data);//*/
	}
	return 0;
}

#endif

int ai_recog_free(void){
	recog.music_num =0;
	recog.error_id = 0;
	recog.error_type = AI_ERROR_NONE;

	recog.domain = RECOG_DOMAIN_NULL;
	free(recog.input);
	recog.input = NULL;
	free(recog.output);
	recog.output = NULL;
	free(recog.volume);
	recog.volume = NULL;
	free(recog.object);
	recog.object = NULL;
	free(recog.operation);
	recog.operation = NULL;
	free(recog.device);
	recog.device = NULL;
 	free(recog.location);
	recog.location = NULL;

	free(recog.search_artist);
	recog.search_artist= NULL;
	free(recog.search_title);
	recog.search_title= NULL;

	recog.music_num =0;
	free(recog.music.artist);
	recog.music.artist = NULL;
	free(recog.music.title);
	recog.music.title = NULL;
	free(recog.music.url);
	recog.music.url = NULL;

	free(recog.weather.temperature);
	recog.weather.temperature = NULL;
	free(recog.weather.weather);
	recog.weather.weather= NULL;
	free(recog.weather.wind);
	recog.weather.wind= NULL;
	free(recog.weather.area);
	recog.weather.area= NULL;

	free(recog.movie.name);
	recog.movie.name = NULL;
	free(recog.movie.sequence);
	recog.movie.sequence= NULL;
	free(recog.movie.director);
	recog.movie.director = NULL;
	free(recog.movie.player);
	recog.movie.player= NULL;
	free(recog.movie.type);
	recog.movie.type= NULL;
	free(recog.movie.area);
	recog.movie.area= NULL;

	free(recog.date);
	recog.date = NULL;
	free(recog.time);
	recog.time = NULL;
	free(recog.event);
	recog.event = NULL;

//--------------------- for sds
//	free(recog.env);
//	recog.env = NULL;
//	free(recog.contextId);
//	recog.contextId = NULL;

}

int ai_to_mozart(void)
{
	asr_mode_cfg = vr_speech_callback_pointer(&recog);
//	recog.status = recog.next_status;
//exit_error:
//	ai_recog_free();
	return 0;
}

int ai_aiengine_init(void)
{
	int ret = -1;
	int error = -1;
	char buf[TMP_BUFFER_SZ] = {0};
	char *pcBuf = NULL;
	cJSON *cjs = NULL;
	cJSON *result = NULL;
/*	if (is_aiengine_init){
		PERROR("Error: aiengine has inited, no allow to init again!\n");
		return 0;
	}	//

	/* print version info */
	DEBUG("%s\n", version);

	/* create AIEngine */
	agn = (struct aiengine *)aiengine_new(agn_cfg);
	if(NULL == agn)
	{
		PERROR("create aiengine error. \n");
		goto exit_error;
	}

	/* auth process */
	aiengine_opt(agn, 10, buf, TMP_BUFFER_SZ);

	DEBUG("Get opt: %s\n", buf);
	cjs = cJSON_Parse(buf);
	if (cjs)
	{
		result = cJSON_GetObjectItem(cjs, "success");
		if (result)
		{
			error = 0;
			ret = 0;
			DEBUG("注册码验证成功！\n");
		}
		cJSON_Delete(cjs);
	}

	if(ret)
	{
		aiengine_opt(agn, 11, buf, TMP_BUFFER_SZ);
		PERROR("%s\n", buf);
	}
	error = 0;

	/* create echo wakeup engine */
	ew = echo_wakeup_new(ew_cfg);
	if(NULL == ew)
	{
		PERROR("Create AEC error. \n");
		goto exit_error;
	}
//	is_aiengine_init= true;

exit_error:
	return error;//*/
}


int ai_status_aecing(void){
	if ((ai_speech_get_status() != VR_SPEECH_INIT)
		||(is_vr_speech_work_flag == false)){
		return -1;
	}

//	ai_cloud_sem_stop();
	ai_to_mozart();
	ai_recog_free();

	if (ai_aec(ew) == 0){
		#if AI_CONTROL_MOZART	  // remove tone when wakeup
			mozart_key_ignore_set(true);
		#endif
		recog.status = AIENGINE_STATUS_SEM;
		return 0;
	}
	else{
		recog.status = AIENGINE_STATUS_ERROR;
		recog.error_type = AI_ERROR_SYSTEM;
		PERROR("AEC error!\n");
		return -1;
	}
}


bool ai_is_play_music(void){
	return aitalk_play_music;
}
int ai_status_seming(void){
	int ret = 0;
	int vol = 0;
#if AI_CONTROL_MOZART
	vol = mozart_volume_get();
	if (vol == 0){
		ai_aitalk_send(aitalk_send_set_volume("10"));	//*/
		usleep(100000);
	}
	if (mozart_module_is_playing()==1){
		ai_aitalk_send(aitalk_send_pause(NULL));
	}
	//ai_tone_time_end();
#endif

#if AI_CONTROL_MOZART
	//mozart_key_ignore_set(true);
#endif

	ai_to_mozart();
#if AI_CONTROL_MOZART	  // remove tone when wakeup
	//mozart_prompt_tone_key_sync("welcome", false);
#endif

	ret = ai_cloud_sem(agn);
	switch (ret){
		case SEM_SUCCESS:
			recog.status = AIENGINE_STATUS_PROCESS;
			break;
		case SEM_EXIT:
			recog.status = AIENGINE_STATUS_AEC;
			break;
		case SEM_NET_LOW:
			recog.error_type = AI_ERROR_NET_SLOW;
			recog.status = AIENGINE_STATUS_ERROR;
			break;
		default:
			recog.error_type = AI_ERROR_SERVER_BUSY;
			recog.status = AIENGINE_STATUS_ERROR;
			break;
	}
	return ret;
}

#if 0
void *ai_seming(void *arg){
	pthread_detach(pthread_self());
	int ret = 0;
	int vol = 0;
	if ((ai_speech_get_status() != VR_SPEECH_INIT)
		||(is_vr_speech_work_flag == false)){
		PERROR("ERROR: not init \n");
		return NULL;
	}

	ai_aec_stop();

	DEBUG("start seming ... \n");
	is_seming = true;
	recog.status = AIENGINE_STATUS_SEM;
#if AI_CONTROL_MOZART
#if 1
	vol = mozart_volume_get();
	if (vol == 0){
		ai_aitalk_send(aitalk_send_set_volume("10"));	//*/
		usleep(100000);
	}
	if (mozart_module_is_playing()==true){
		ai_aitalk_send(aitalk_send_pause(NULL));
	}
#endif
	//ai_tone_time_end();
#endif

#if AI_CONTROL_MOZART
//	mozart_key_ignore_set(true);
#endif

	ai_to_mozart();
#if AI_CONTROL_MOZART	  // remove tone when wakeup
	//mozart_prompt_tone_key_sync("welcome", false);
#endif

	ret = ai_cloud_sem(agn);
	switch (ret){
		case SEM_SUCCESS:
			recog.status = AIENGINE_STATUS_PROCESS;
			break;
		case SEM_EXIT:
			recog.status = AIENGINE_STATUS_AEC;
			break;
		case SEM_NET_LOW:
			recog.error_type = AI_ERROR_NET_SLOW;
			recog.status = AIENGINE_STATUS_ERROR;
			break;
		default:
			recog.error_type = AI_ERROR_SERVER_BUSY;
			recog.status = AIENGINE_STATUS_ERROR;
			break;
	}
	ai_to_mozart();
	DEBUG("stop seming ... \n");
	is_seming = false;
	return NULL;
}

int ai_sem_start(void){
	int count =0;
//-------------------- wate last seming stop;
	if (is_seming){
		ai_cloud_sem_stop();
	}
	while(is_seming){
		count ++;
		usleep(1000);
		if (count   > 1000){
			PERROR("Error: is_seming \n");
			is_seming = false;
			break;
		}
	}

	pthread_t ai_sem_thread;
	if (pthread_create(&ai_sem_thread, NULL, ai_seming, NULL) != 0) {
		PERROR("Can't create ai_sem_thread in : %s\n",strerror(errno));
	}
	return 0;
}

#endif

int ai_status_process(void){
	ai_to_mozart();
	if (ai_server_fun(&recog) == 0){
	#ifdef SYN_TOO_LONG
		ai_sem_time(recog.input);
	#endif
	}
	else{
		recog.status = AIENGINE_STATUS_ERROR;
	}
	if (recog.error_id){
		PERROR("Error ID = %d\n",recog.error_id);
		recog.error_type = ai_error_get_id(recog.error_id);
		recog.status = AIENGINE_STATUS_ERROR;
	}
}

int ai_status_error(void){
	DEBUG("ERROR   [%d]: %s\n",recog.error_type,error_type[recog.error_type]);//*/
	if ((recog.error_type <= AI_ERROR_NONE)&&(recog.error_type > AI_ERROR_MAX)){
		return 0;
	}

	if (recog.recordId != NULL){
		PERROR("LGO_ERR: %s, %s\n",recog.recordId,error_type[recog.error_type]);//*/

	#ifdef SYN_TOO_LONG
		ai_log_add(LOG_ERROR,"%s, %s",recog.recordId,error_type[recog.error_type]);//*/
	#endif
	}
	else{
		PERROR("LGO_ERR: %s\n",error_type[recog.error_type]);//*/
	#ifdef SYN_TOO_LONG
		ai_log_add(LOG_ERROR,"%s",error_type[recog.error_type]);//*/
	#endif
	}
	switch (recog.error_type){
	case  AI_ERROR_SEM_FAIL_1:
		recog.status = AIENGINE_STATUS_SEM;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_sem_fail_1", false);
		#endif
		break;
	case AI_ERROR_SEM_FAIL_2:
		recog.status = AIENGINE_STATUS_SEM;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_sem_fail_2", false);
		#endif
		break;
	case AI_ERROR_SEM_FAIL_3:
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_sem_fail_3", false);
		#endif
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_ATUHORITY:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_authority", false);
		#endif
		break;
	case  AI_ERROR_INVALID_DOMAIN:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_invalid_domain", false);
		#endif
		break;
	case  AI_ERROR_SYSTEM:
		recog.status = AIENGINE_STATUS_EXIT;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_system", false);
		#endif
		break;
	case  AI_ERROR_NO_VOICE:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_no_voice", false);
		#endif
		break;
	case  AI_ERROR_SERVER_BUSY:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_server_busy", false);
		#endif
		break;
	case  AI_ERROR_NET_SLOW:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_net_slow", false);
		#endif
		break;
	case  AI_ERROR_NET_FAIL:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_net_fail", false);
		#endif
		break;
	default:
		recog.status = AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("error_net_slow", false);
		#endif
		break;
	}
	ai_to_mozart();
	ai_recog_free();
	return 0;
}

#if 0
int ai_aiengine_restart(void){
	int error = 0;
	DEBUG("ai_aiengine_restart      !...\n");
	if ((ai_cloudsem_restart() == -1) || (ai_aec_restart() == -1)){
		recog.status = AIENGINE_STATUS_ERROR;
		recog.next_status = AIENGINE_STATUS_ERROR;
		error = -1;
		PERROR("Error:    ai_aiengine_restart \n");
		goto exit_error;
	}
	ai_aec_start();
	recog.next_status = AIENGINE_STATUS_AEC;
	recog.status = AIENGINE_STATUS_AEC;
exit_error:
	return error;
}//*/
#endif
int ai_init_data(void){
	recog.status = AIENGINE_STATUS_INIT;
//	recog.next_status = AIENGINE_STATUS_AEC;
	recog.state =	 SDS_STATE_NULL;
	recog.domain = RECOG_DOMAIN_NULL;
//	recog.is_get_song_recommend  =false;
//	recog.is_play_song_recommend  =false;
	recog.input = NULL;
	recog.output = NULL;
	recog.object = NULL;
	recog.operation = NULL;
	recog.device= NULL;
	recog.location= NULL;
	recog.volume = NULL;
	recog.music_num =0;
	recog.music.artist = NULL;
	recog.music.title = NULL;
	recog.music.url = NULL;
	recog.weather.temperature= NULL;
	recog.weather.weather= NULL;
	recog.weather.wind= NULL;
	recog.weather.area= NULL;
	recog.event= NULL;
	recog.search_title= NULL;
	recog.search_artist= NULL;
//	recog.tts_enable = 0;

	recog.error_id = 0;
	recog.error_type = 0;
	recog.recordId = NULL;
	recog.env = NULL;
	recog.contextId = NULL;
	recog.sds_flag = 0;
    recog.date = NULL;
	recog.time = NULL;
	recog.event = NULL;
}

int ai_init(void){
	int err = 0;
	ai_init_data();
	asr_quit_finish = 0;
	is_aiengine_enable = false;
	is_vr_speech_work_flag = false;
	if (ai_aiengine_init() == -1){
		PERROR("Ai aiengine init falsed!\n");
		err = -1;
		goto exit_error;
	}
	ai_server_init();
exit_error:
	if (err){
		recog.status = AIENGINE_STATUS_ERROR;
		recog.error_type = AI_ERROR_SYSTEM;
	//	ai_to_mozart();
	}
	else{
		recog.status = AIENGINE_STATUS_AEC;
		is_vr_speech_work_flag = true;
		is_vr_speech_work_flag = true;
	}
	return err;
}

#if 1
int ai_set_enable(bool enable){
	if(enable == true){
		DEBUG("=========================== start aiengine ...\n");
		if (is_aiengine_init == false){
			recog.status =AIENGINE_STATUS_INIT;
		}else{
			ai_server_restart();
			recog.status =AIENGINE_STATUS_AEC;
		}

		recog.status =AIENGINE_STATUS_AEC;
		#if AI_CONTROL_MOZART
		//mozart_prompt_tone_key_sync("atalk_hi_12", true);
		#endif
		is_aiengine_enable = true;
	}
	else{
		if (is_aiengine_enable){
			DEBUG("=========================== stop aiengine ...\n");
			is_aiengine_enable = false;
			ai_aec_stop();
			ai_cloud_sem_stop();
			ai_tts_stop();
		}
	}
	return 0;
}

#else
int ai_set_enable(bool enable){
	if (is_vr_speech_work_flag){
		if(enable == true){
			DEBUG("=========== start aec ...\n");
			is_aiengine_enable = true;
		}
		else{
			if (is_aiengine_enable){
				DEBUG("=================== stop aec ...\n");
				is_aiengine_enable = false;
				ai_aec_stop();
			}
		}
	}
	return 0;
}
#endif

#if 0
void *ai_aec_run(void *arg){
	pthread_detach(pthread_self());
    while(is_vr_speech_work_flag){
		while(is_aec_enable){
			ai_aecing();
			printf(".");
			usleep(10000);
		}
		printf(">");
		usleep(10000);
    }

	ai_aec_stop();
	if (ew){
		echo_wakeup_delete(ew);
		ew = NULL;
	}
	return;
}
#endif


int ai_aiengine_start(void){
	DEBUG("aiengine start!...\n");
	ai_set_enable(true);
	ai_song_recommend_auto();
	return 0;
}

int ai_aiengine_stop(void){
	DEBUG("aiengine stop!...\n");
	ai_set_enable(false);
	return 0;
}

int ai_aiengine_exit(void){
	DEBUG("aiengine exit!...\n");
	ai_aiengine_stop();
	ai_cloud_sem_free();
	if (ew){
		echo_wakeup_delete(ew);
		ew = NULL;
	}
	if (agn){
		aiengine_delete(agn);
		agn = NULL;
	}
	free(recog.env);
	recog.env = NULL;
	free(recog.contextId);
	recog.contextId = NULL;
	free(recog.recordId);
	recog.recordId = NULL;

	recog.status = AIENGINE_STATUS_INIT;
	DEBUG("aiengine exit finished     !...\n");
	return 0;
//	is_aiengine_init = false;
}

#if 1
int recog_status_last = 0;

void *ai_run(void *arg){
	pthread_detach(pthread_self());
	int error = 0;
//	DEBUG("=========================== ai_run: %s\n", aiengineStatus[recog.status]);
	ai_init_data();
//	DEBUG("=========================== ai_run: %s\n", aiengineStatus[recog.status]);
    while(is_vr_speech_work_flag){
		while(is_aiengine_enable){
			if((recog.status_last !=    recog.status)
			 &&(recog.status < AIENGINE_STATUS_MAX)){
				DEBUG("=================== AIENGINE STATUS: %s\n", aiengineStatus[recog.status]);
			}
			switch(recog.status){
				case AIENGINE_STATUS_INIT:
					if (ai_init() != 0){
						error = -1;
						PERROR("ERROR: aiengine init !\n");
						goto exit_error;
					}
					break;
  				case AIENGINE_STATUS_AEC:
			//		DEBUG("=========================== AIENGINE_STATUS_AEC ...\n");
					ai_status_aecing();
					break;
				case AIENGINE_STATUS_SEM:
			//		DEBUG("=========================== AIENGINE_STATUS_SEM ...\n");
				//	ai_to_mozart();
					ai_status_seming();
					break;
				case AIENGINE_STATUS_PROCESS:
			//		DEBUG("=========================== AIENGINE_STATUS_PROCESS ...\n");
					ai_status_process();
				//	ai_to_mozart();
					break;
				case AIENGINE_STATUS_ERROR:
					DEBUG("=========================== AIENGINE_STATUS_ERROR ...\n");
					ai_status_error();
					break;
				case AIENGINE_STATUS_STOP:
					ai_aiengine_stop();
					break;
				case AIENGINE_STATUS_EXIT:
					DEBUG("=========================== AIENGINE_STATUS_EXIT ...\n");
					is_aiengine_enable = false;
					is_vr_speech_work_flag = false;
					break;
				default:
					recog.status = AIENGINE_STATUS_AEC;
					break;	//*/
			}
			printf(".");
			usleep(10000);
		}
		printf(">");
		usleep(10000);
    }
exit_error:
	if (error){
		recog.error_type = AI_ERROR_SYSTEM;
		ai_status_error();
	//	ai_to_mozart();
	}
	ai_exit();
	pthread_exit(&error);
}

int ai_exit(void){
	ai_aiengine_exit();
//	ai_aec_destory();
//	ai_cloudsem_destory();
	ai_server_exit();
	ai_recog_free();
	asr_quit_finish = 1;
#ifdef SYN_TOO_LONG
	ai_log_add(LOG_DEBUG,"ai_asr_runing thread exit");//*/
#endif
}
#endif
/*
int ai_tts(char *data){
	int ret = 0;
	ret = ai_cloud_tts(agn,data);
	ai_tts_time(data);
	return ret;
}
//*/
int ai_tts(char *data,int enable_stop){
#if 1
//	printf("TTS: %s\n",data);
	if (ai_cloud_tts(agn,data) == -1){
		return -1;
	}
#ifdef SYN_TOO_LONG
	ai_tts_time(data);
#endif
	if (enable_stop == true){
		#if AI_CONTROL_MOZART
		music_info music;
		music.url ="/tmp/cloud_sync_record.mp3";
		music.artist = NULL;
		music.title = NULL;
		ai_aitalk_send(aitalk_send_play_url(&music));
		#else
		system("mplayer /tmp/cloud_sync_record.mp3");
		#endif
	}
	else{
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("ai_cloud_sync", false);
		#else
		system("mplayer /tmp/cloud_sync_record.mp3");
		#endif
	}
#else
	char *tts_url=NULL;
    tts_url=ai_output_url(data);
    //mozart_player_playurl(NULL,tts_url);
    mozart_prompt_tone_play_sync(tts_url,enable_stop);
    // __mozart_prompt_tone_play_sync(tts_url);
   // ai_aitalk_send(tts_url);
    //ai_aitalk_send(aitalk_send_play_url(tts_url));
	//ai_tts_time(data);
#endif
	return 0;
}

#if 0
int mozart_key_wakeup(void){
	DEBUG("mozart_key_wakeup start...\n");
	ai_aec_stop();
/*	if(is_aiengine_enable){
		switch(recog.status){
			case AIENGINE_STATUS_AEC:
				break;
			case AIENGINE_STATUS_SEM:
			//	ai_cloud_sem_stop();
				break;
			default:
				break;
		}
	}//*/
	return 0;
}
#endif

void ai_speech_set_status(vr_speech_status_type status){
	vr_speech_status = status;
}

int ai_speech_get_status(){
	return vr_speech_status;
}

/*
 * usage : ./demo /path/to/libaiengine.so w/s/t
 */
 extern sem_t sem_ai_startup;
int ai_speech_startup(int wakeup_mode, mozart_vr_speech_callback callback)
{
	sem_wait(&sem_ai_startup);
	pthread_t voice_recog_thread;
	if (ai_speech_get_status() != VR_SPEECH_NULL){
		ai_aiengine_exit();
		ai_speech_set_status(VR_SPEECH_NULL);
	}

	if(ai_init()== -1){
		PERROR("AI init error!...\n");
		goto exit_error;
	}
	DEBUG("vr speech     asr start!...\n");
	vr_speech_callback_pointer = callback;
	if (pthread_create(&voice_recog_thread, NULL, ai_run, NULL) != 0) {
		PERROR("Can't create voice_recog_thread in : %s\n",strerror(errno));
		goto exit_error;
	}

	ai_speech_set_status(VR_SPEECH_INIT);
exit_error:
	DEBUG("mozart_vr_speech_startup finish.\n");
	sem_post(&sem_ai_startup);
	return 0;
}

int ai_speech_shutdown(void){
	sem_wait(&sem_ai_startup);
	//wait for record_input quit.

//	pthread_mutex_lock(&ai_mutex);
	int status = ai_speech_get_status();

	DEBUG("vr speech shutdown!...\n");
	switch (status){
		case VR_SPEECH_NULL:
			DEBUG("vr_speech is not startup!\n");
			return 0;
			break;
		case VR_SPEECH_ERROR:
			DEBUG("vr_speech running wrong or vr_speech quit!\n");
			break;
		case VR_SPEECH_INIT:
			DEBUG("vr_speech shutdown!\n");
			ai_speech_set_status(VR_SPEECH_QUIT);
			ai_aiengine_exit();
		/*	int timeout =0;
			while(!asr_quit_finish){
				usleep(1000);
				if (++timeout > 2000){
						break;
				}
			}	//*/
			break;
		default:
			DEBUG("vr_speech unknown status!\n");
			return 0;
			break;
	}
//    mozart_soundcard_uninit(record_info);
/*	if(0 != share_mem_set(VR_DOMAIN, RESPONSE_CANCEL)){
		PERROR("share_mem_set STATUS_SHUTDOWN failure.\n");
	}	//*/
	ai_speech_set_status(VR_SPEECH_NULL);
//	pthread_mutex_unlock(&ai_mutex);
	sem_post(&sem_ai_startup);
	return 0;
}

