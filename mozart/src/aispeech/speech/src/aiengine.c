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
#include "ai_server.h"
#include "baselib.h"

#include "ini_interface.h"

music_obj *global_music;
struct op *global_op;

#if AI_CONTROL_MOZART
#include "mozart_key.h"
#endif

#if AI_CONTROL_MOZART_ATALK
#include "mozart_aitalk.h"
#endif

#include "echo_wakeup.h"

#include <semaphore.h>
static const char *version =
"\n=============================\n"\
"AIHOME: Aitalk for DS1825-pro\n"\
"Ver : 1.1.0\n"\
"Date: 2016-09-05\n"\
"=============================\n";

//		\"server\": \"ws://s-test.api.aispeech.com:10000\"\
//		\"server\": \"ws://112.80.39.95:8009\"\

static const char *ew_cfg =
"\
{\
    \"luaPath\": \"/usr/fs/usr/share/vr/bin/luabin.lub\",\
    \"appKey\": \"14327742440003c5\",\
    \"secretKey\": \"59db7351b3790ec75c776f6881b35d7e\",\
    \"provision\": \"/usr/fs/usr/share/vr/bin/aiengine-2.8.8-14327742440003c5.provision\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"/mnt/sdcard/a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"/usr/fs/usr/share/vr/bin/vad.aihome.v0.5.20160324.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"native\": {\
        \"cn.dnn\": {\
            \"resBinPath\": \"/usr/fs/usr/share/vr/bin/wakeup_aihome_aispeech_nhxl_20161019.bin\"\
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
    \"provision\": \"/usr/fs/usr/share/vr/bin/aiengine-2.8.8-14327742440003c5.provision\",\
    \"serialNumber\": \"/usr/data/serialNumber\",\
    \"prof\": {\
        \"enable\": 0,\
        \"output\": \"/mnt/sdcard/a.log\"\
    },\
    \"vad\":{\
        \"enable\": 1,\
        \"res\": \"/usr/fs/usr/share/vr/bin/vad.aihome.v0.5.20160324.bin\",\
        \"speechLowSeek\": 70,\
        \"sampleRate\": 16000,\
        \"pauseTime\":700,\
        \"strip\": 1\
    },\
    \"cloud\": {\
		\"server\": \"ws://s-test.api.aispeech.com:10000\"\
    }\
}";

ai_status_s ai_flag;
vr_info recog;
pthread_mutex_t ai_lock = PTHREAD_MUTEX_INITIALIZER;

struct timeval t_debug;
struct aiengine *agn = NULL;
echo_wakeup_t *ew = NULL;
ini_aiengine_s aiengine_ini;


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
}//*/
int ai_sem_time(char *data){
	if (data == NULL)
		return -1;

	unsigned   long time = 0;
	time_subtract(&time,t_sem_start,t_sem_end);
	printf("sem: %6d\n",time);
	if ((recog.recordId)){//&&(time > 1500)){
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
	free(recog.semantics);
	recog.semantics = NULL;

	free(recog.scene);
	recog.scene = NULL;
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
	ai_flag.asr_mode_cfg = ai_flag.vr_callback_pointer(&recog);
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
	while(ai_song_list.is_getting == true){
		usleep(10000);
	}
	system("echo 3 > /proc/sys/vm/drop_caches");
	ai_to_mozart();
	if (aitalk_cloudplayer_is_playing()){
	//	DEBUG("aitalk   is playing..! %d \n", recog.is_control_play_music);
		if(recog.is_control_play_music == false){
			usleep(10000);
			ai_aitalk_send(aitalk_send_resume(false));	//*/
			usleep(10000);
		}
	}
	else{
		DEBUG("aitalk    is not playing..!\n");
	}
	#if AI_CONTROL_MOZART	  // remove tone when wakeup
		mozart_key_ignore_set(false);
	#endif
	recog.is_control_play_music = false;
	ai_recog_free();
	if (ai_aec(ew) == 0){
		if(ai_flag.is_running){
			if(ai_song_list.is_getting == true){
				ai_aitalk_send(aitalk_send_error("error_server_busy"));
				recog.status = AIENGINE_STATUS_AEC;
			}
			else{
				#if AI_CONTROL_MOZART	  // remove tone when wakeup
					mozart_key_ignore_set(true);
				#endif
				recog.status = AIENGINE_STATUS_SEM;
			}
		}
		return 0;
	}
	else{
		recog.status = AIENGINE_STATUS_ERROR;
		recog.error_type = AI_ERROR_SYSTEM;
		PERROR("AEC error!\n");
		return -1;
	}
}

int ai_status_seming(void){
	int ret = 0;
	int vol = 0;
	system("echo 3 > /proc/sys/vm/drop_caches");
#if AI_CONTROL_MOZART
	vol = mozart_volume_get();
	if (vol == 0){
		ai_aitalk_send(aitalk_send_set_volume("10",""));	//*/
		usleep(100000);
	}
	if (aitalk_cloudplayer_is_playing()){
		ai_aitalk_send(aitalk_send_pause(false));
	}
	//ai_tone_time_end();
#endif
	ai_to_mozart();

	ret = ai_cloud_sem(agn);
	if(ai_flag.is_running){
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
	}
	else{
		recog.status = AIENGINE_STATUS_AEC;
	}
	return ret;
}


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
		recog.error_id = 0;
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
		ai_aitalk_send(aitalk_send_error("error_sem_fail_1"));
		recog.status = AIENGINE_STATUS_SEM;
		break;
	case AI_ERROR_SEM_FAIL_2:
		ai_aitalk_send(aitalk_send_error("error_sem_fail_2"));
		recog.status = AIENGINE_STATUS_SEM;
		break;
	case AI_ERROR_SEM_FAIL_3:
		ai_aitalk_send(aitalk_send_error("error_sem_fail_3"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_ATUHORITY:
		ai_aitalk_send(aitalk_send_error("error_authority"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_INVALID_DOMAIN:
		ai_aitalk_send(aitalk_send_error("error_invalid_domain"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_SYSTEM:
		ai_aitalk_send(aitalk_send_error("error_server_busy"));
		sleep(2);
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_NO_VOICE:
		ai_aitalk_send(aitalk_send_error("error_no_voice"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_SERVER_BUSY:
		ai_aitalk_send(aitalk_send_error("error_server_busy"));
		sleep(2);
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_NET_SLOW:
		ai_aitalk_send(aitalk_send_error("error_net_slow_again"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	case  AI_ERROR_NET_FAIL:
		ai_aitalk_send(aitalk_send_error("error_net_fail"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	default:
		ai_aitalk_send(aitalk_send_error("error_net_slow"));
		recog.status = AIENGINE_STATUS_AEC;
		break;
	}
	ai_to_mozart();
	ai_recog_free();
	return 0;
}

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
	recog.semantics = NULL;
	recog.scene= NULL;
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
	int retvalue = 1;
	int err = 0;
	aiengine_init_get_config(&aiengine_ini);
	ai_init_data();
	ai_flag.is_running = false;
	if (ai_aiengine_init() == -1){
		PERROR("Ai aiengine init falsed!\n");
		err = -1;
		goto exit_error;
	}
	ai_flag.is_init = true;

	/*musci list init*/
	music_list_alloc(&global_music, 20);

	/*file operate init*/
	int fd = file_create("/usr/data/music_list.json");
	if (fd == -1) {
		print("error\n\n");
		retvalue = -1;
		exit(0);
	}
	op_init(&global_op, fd, global_music);
	op_reg_low_output(global_op, low_output_cb);
	op_reg_high_output(global_op, high_output_cb);
	op_reg_low_input(global_op, low_input_cb);
	op_reg_cur_output(global_op, cur_output_cb);

	machine_open(global_op);

	ai_server_init();
exit_error:
	if (err){
		recog.status = AIENGINE_STATUS_ERROR;
		recog.error_type = AI_ERROR_SYSTEM;
	//	ai_to_mozart();
	}
	else{
		recog.status = AIENGINE_STATUS_AEC;
	}
	return err;
}

int ai_set_enable(bool enable){
	if(enable == true){
		DEBUG("=========================== start aiengine ...\n");
	//	if (is_aiengine_init == false){
	//		recog.status =AIENGINE_STATUS_STOP;
	//	}else{
			recog.status =AIENGINE_STATUS_AEC;
			ai_flag.is_running = true;
			recog.key_record_stop = false;
			ai_song_list_set_enable(true);
			usleep(10000);
			ai_aitalk_send(aitalk_send_current_music(false));	//*/
			usleep(10000);
			system("echo 3 > /proc/sys/vm/drop_caches");
		//	if (ai_flag.is_init){
		//		ai_server_restart();
		//		ai_song_recommend_auto();
		//	}
	//	}
	}
	else{
		if (ai_flag.is_running){
			DEBUG("=========================== stop aiengine ...\n");
			ai_flag.is_running = false;
			if (ai_flag.is_init){
				ai_aiengine_exit();
			}
			recog.key_record_stop = false;
			ai_song_list_set_enable(false);
			system("echo 3 > /proc/sys/vm/drop_caches");
		}
	}
	return 0;
}


int ai_aiengine_start(void){
	DEBUG("aiengine start!...\n");
	ai_set_enable(true);
	return 0;
}

int ai_aiengine_stop(void){
	ai_set_enable(false);
	return 0;
}

int ai_aiengine_delete(void){
	if (ai_flag.is_init){
		if (ew){
			echo_wakeup_delete(ew);
			ew = NULL;
		}

		if (agn){
			aiengine_delete(agn);
			agn = NULL;
		}
	}
	ai_flag.is_init = false;
	ai_server_exit();
	return 0;
}


int ai_aiengine_exit(void){
	DEBUG("aiengine exit!...\n");
	ai_flag.is_running = false;
	ai_aec_stop();
	ai_cloud_sem_stop();
	ai_tts_stop();
	ai_cloud_sem_free();
	ai_cloud_sem_text_stop();
	ai_recog_free();
	free(recog.env);
	recog.env = NULL;
	free(recog.contextId);
	recog.contextId = NULL;
	free(recog.recordId);
	recog.recordId = NULL;

	recog.status = AIENGINE_STATUS_INIT;
	ai_server_exit();
	DEBUG("aiengine exit finished     !...\n");
	return 0;
//	is_aiengine_init = false;
}

#define AI_TEST_THREAD 0
#if AI_TEST_THREAD
void *ai_test_run(void *arg){
	int count =0;
	DEBUG("Start ai_test_run !... \n");
	sleep(20);
	while(ai_flag.is_working){
			ai_aiengine_start();
			sleep(1);
			ai_aiengine_stop();
			sleep(1);
			count ++;
		//	if ((count %100) == 0){
				DEBUG("count = %8d\n",count);
		//	}
	}
}
#endif

void *ai_run(void *arg){
	pthread_detach(pthread_self());
	int error = 0;
	ai_flag.is_working = true;
	while(ai_flag.is_working){
		while(ai_flag.is_running){
			if((recog.status_last !=    recog.status)
			 &&(recog.status < AIENGINE_STATUS_MAX)){
				DEBUG("=================== AIENGINE STATUS: %s\n", aiengineStatus[recog.status]);
			}
			switch(recog.status){
			//	case AIENGINE_STATUS_INIT:
			/*		if (ai_init() != 0){
						error = -1;
						PERROR("ERROR: aiengine init !\n");
						goto exit_error;
					}//*/
			//		break;
  				case AIENGINE_STATUS_AEC:
					ai_status_aecing();
					break;
				case AIENGINE_STATUS_SEM:
					ai_status_seming();
					break;
				case AIENGINE_STATUS_PROCESS:
					ai_status_process();
					break;
				case AIENGINE_STATUS_ERROR:
					ai_status_error();
					break;
			//	case AIENGINE_STATUS_STOP:
				//	ai_aiengine_stop();
			//		break;
			//	case AIENGINE_STATUS_EXIT:
			//		ai_flag.is_running = false;
			//		ai_flag.is_working = false;
			//		break;
				default:
					recog.status = AIENGINE_STATUS_AEC;
					break;	//*/
			}
			printf("*");
			usleep(10000);
		}
	//printf(">");
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
	ai_flag.is_running = false;
	ai_flag.is_working = false;
	ai_aiengine_exit();
//	ai_server_exit();
//	ai_recog_free();
#ifdef SYN_TOO_LONG
	ai_log_add(LOG_DEBUG,"ai_asr_runing thread exit");//*/
#endif
}

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
	//	#if AI_CONTROL_MOZART
		ai_aitalk_send(aitalk_send_play_tts("/tmp/cloud_sync_record.mp3"));
	//	#else
	//	system("mplayer /tmp/cloud_sync_record.mp3");
	//	#endif
	}
	else{
		#if AI_CONTROL_MOZART
		mozart_prompt_tone_key_sync("ai_cloud_sync",false);
	//	#else
	//	system("mplayer /tmp/cloud_sync_record.mp3");
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

int ai_key_record_wakeup(void){
	int count = 0;
	DEBUG("ai_key_record_wakeup start...\n");
	//if ((ai_flag.is_running)&&(ai_flag.is_init)){
	//	if(recog.status == AIENGINE_STATUS_AEC){
			ai_aec_stop();
		//	while(recog.status == AIENGINE_STATUS_AEC){
		//		usleep(1000); // wake 15s to deal
		//		count ++;
		//		if (count > 5000){
		//			break;
		//		}
		//	}
	//	}
	//}
}

int ai_key_record_stop(void){
	int count = 0;
	DEBUG("ai_key_record_stop start...\n");
	if ((ai_flag.is_running)&&(ai_flag.is_init)){
		switch(recog.status){
			case AIENGINE_STATUS_SEM:
				recog.key_record_stop = true;
				ai_cloud_sem_stop();
		/*		while((recog.status == AIENGINE_STATUS_SEM)){
					usleep(1000); // wake 15s to deal
					count ++;
					if (count > 5000){
						break;
					}
				}//*/
				break;
			case AIENGINE_STATUS_PROCESS:
				recog.key_record_stop = true;
		/*		while((recog.status == AIENGINE_STATUS_PROCESS)){
					usleep(1000); // wake 15s to deal
					count ++;
					if (count > 5000){
						break;
					}
				}//*/
				break;	//*/
			default:
				break;	//*/
		}
	}
	return 0;
}


void ai_speech_set_status(vr_speech_status_type status){
	ai_flag.vr_status = status;
}

int ai_speech_get_status(){
	return ai_flag.vr_status;
}

/*
 * usage : ./demo /path/to/libaiengine.so w/s/t
 */

int ai_speech_startup(int wakeup_mode, mozart_vr_speech_callback callback)
{
//	sem_wait(&sem_ai_startup);
/*	if (ai_speech_get_status() != VR_SPEECH_NULL){
		ai_aiengine_exit();
		ai_speech_set_status(VR_SPEECH_NULL);
	}	//*/

	if(ai_flag.is_init != true){
		system("echo 3 > /proc/sys/vm/drop_caches");
		if(ai_init()== -1){
			PERROR("AI init error!...\n");
			goto exit_error;
		}

		ai_song_list_init();

		DEBUG("vr speech     asr start!...\n");
		ai_flag.vr_callback_pointer = callback;
		ai_flag.is_working = true;
		pthread_t voice_recog_thread;
		if (pthread_create(&voice_recog_thread, NULL, ai_run, NULL) != 0) {
			PERROR("Can't create voice_recog_thread in : %s\n",strerror(errno));
			goto exit_error;
		}
		#if AI_TEST_THREAD
		pthread_t voice_test_thread;
		if (pthread_create(&voice_test_thread, NULL, ai_test_run, NULL) != 0) {
			PERROR("Can't create voice_test_thread in : %s\n",strerror(errno));
			goto exit_error;
		}
		#endif
	}
	ai_speech_set_status(VR_SPEECH_INIT);
exit_error:
	DEBUG("mozart_vr_speech_startup finish.\n");
//	sem_post(&sem_ai_startup);
	return 0;
}

int ai_speech_shutdown(void){
//	sem_wait(&sem_ai_startup);
	int status = ai_speech_get_status();
	DEBUG("vr speech shutdown!...\n");
	switch (status){
		case VR_SPEECH_NULL:
			DEBUG("vr_speech is not startup!\n");
			break;
		case VR_SPEECH_ERROR:
			DEBUG("vr_speech running wrong or vr_speech quit!\n");
			break;
		case VR_SPEECH_INIT:
			DEBUG("vr_speech shutdown!\n");
			ai_aiengine_exit();
			ai_speech_set_status(VR_SPEECH_NULL);
			break;
		default:
			DEBUG("vr_speech unknown status!\n");
			break;
	}

	DEBUG("vr speech shutdown finish!...\n");
//	sem_post(&sem_ai_startup);
	return 0;
}

