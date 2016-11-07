#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aiengine.h"
#include "ai_sound_dev.h"
#include "cJSON.h"
#include "aiengine_app.h"

extern int fd_dsp_rd;

ai_sem_flag_t ai_sem_flag;

static char *new_semantic_param =NULL;
static char *p_semantic_param = NULL;
static char *semantic_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"vadEnable\": 1,\
    \"app\": {\
        \"userId\": \"wifiBox\"\
    },\
    \"audio\": {\
        \"audioType\": \"wav\",\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"channel\": 1,\
		\"compress\": \"speex\"\
    },\
    \"request\": {\
        \"coreType\": \"cn.sds\",\
        \"res\": \"aihome\",\
 		\"sdsExpand\": {\
        	\"prevdomain\": \"\",\
			\"lastServiceType\": \"cloud\"\
		}\
    }\
}";

int ai_sem_error_count = 0;

static char* ai_cloudsem_get_lbs_city(void);
//static int ai_add_lbs_city_to_sem_param(char *lbs_city);
static int ai_add_sds_info_to_sem_param(void);

int _semantic_callback(const void *usrdata, const char *id, int type,
                                        const void *message, int size)
{
	//cJSON *nbest;
	cJSON *out = NULL;
	cJSON *vad_status = NULL;
	cJSON *result = NULL;
	cJSON *recordId = NULL;
	cJSON *error_j = NULL;
	cJSON *errId_j = NULL;
  //  if(strstr((char *)message, "input")){
//	    DEBUG("sem: size = %d\n%s\n",size,(char *)message);
  //  }
    out = cJSON_Parse((char*) message);
    if (!out)
    {
        return -1;
    }
    //vad status
    vad_status = cJSON_GetObjectItem(out, "vad_status");
  //  nbest = cJSON_GetObjectItem(out, "nbest");
    if (vad_status)  {
		// vad_status  0 - start , 1 -speaking, 2 - end
        if(vad_status->valueint == 2)  {
			ai_sem_flag.speak_end = true;
            printf("*****************vad end**********************\n");
        }
        goto exit_error;
    }
	recordId = cJSON_GetObjectItem(out, "recordId");
	if(recordId){
//		DEBUG("recordId: %s\n", recordId->valuestring);
		free(recog.recordId);
		recog.recordId = NULL;
		recog.recordId = strdup(recordId->valuestring);
	}		//*/

    result = cJSON_GetObjectItem(out, "result");
    if (result)
    {
#ifdef SYN_TOO_LONG
    gettimeofday(&t_sem_end,NULL);
#endif
		if(ai_slot_resolve(&recog,result) == -1){
			ai_sem_error_count++;
			if (ai_sem_error_count == 1){
				recog.error_type = AI_ERROR_SEM_FAIL_1;
				recog.status    = AIENGINE_STATUS_ERROR;
				goto exit_error;
			}
			else if (ai_sem_error_count == 2){
				recog.error_type = AI_ERROR_SEM_FAIL_2;
				recog.status    =AIENGINE_STATUS_ERROR;
				goto exit_error;
			}
			else{
				ai_sem_error_count = 0;
				recog.error_type = AI_ERROR_SEM_FAIL_3;
				recog.status    =AIENGINE_STATUS_ERROR;
				goto exit_error;
			}
			PERROR("Error json!\n");
			ai_sem_flag.state = SEM_STATUS_FAIL;
			goto exit_error;
		}	//*/
		ai_sem_error_count =0;
		ai_sem_flag.state = SEM_STATUS_SUCCESS;
			//	recog.status = AIENGINE_STATUS_SEM_STATUS_SUCCESS;
		//	}
    }

    error_j = cJSON_GetObjectItem(out, "error");
    if (error_j)
    {
		char *error_s = cJSON_Print(error_j);
		DEBUG("CLOUD SEM Error: \n%s\n", error_s);
		free(error_s);
 		errId_j = cJSON_GetObjectItem(out, "errId");
		if (errId_j){
			recog.error_id = errId_j->valueint;
    	    PERROR("CLOUD SEM Error ID = %d\n", recog.error_id);
		}
		recog.status = AIENGINE_STATUS_ERROR;
		ai_sem_flag.state = SEM_STATUS_FAIL;
    //    DEBUG("CLOUD SEM Error: \n%s\n", cJSON_Print(error));
    }

exit_error:
    if (out)
    {
        cJSON_Delete(out);
    }

    return 0;
}


int  ai_cloud_sem_stoping(void){
	int timeout = 0;
	while(ai_sem_flag.state != SEM_STATUS_IDEL){
		timeout ++;
		usleep(1000);		//	100ms*50 = 5s
		if (++timeout > 20000){
			PERROR("ERROR: Stop cloud sem time out!\n");
			break;
		}
	}
	return 0;
}

void ai_cloud_sem_stop(void){
	DEBUG("========> Stop cloud sem!\n");
	if(ai_sem_flag.state != SEM_STATUS_IDEL){
		ai_sem_flag.set_end = true;
		ai_cloud_sem_stoping();
	}
	DEBUG("========> Stop cloud sem finish!\n");
}

void ai_cloud_sem_free(void){
	free(new_semantic_param);
	new_semantic_param = NULL;
}

int ai_cloud_sem(struct aiengine *agn)
{
    char uuid[64] = {0};
	const void *usrdata  ;//= NULL;
    char buf[RECORD_BUFSZ] = {0};
    int loop = 0;
    int ret = 0;
	int buf_count = 0;
	ai_mutex_lock();
	ai_sem_flag.state = SEM_STATUS_START;
	ai_sem_flag.error = false;
	ai_sem_flag.set_end = false;
	ai_sem_flag.speak_end = false;
	ai_sem_flag.result = SEM_SUCCESS;

	if(1 == recog.sds_flag){
		//update_sdsExpand_info();
		//p_semantic_param = new_semantic_param;
        if(ai_add_sds_info_to_sem_param()<0){
            PERROR("add sds info to sem param failed!\n");
        }else{
		    p_semantic_param = new_semantic_param;
        }
		recog.sds_flag =0;
	}else{
		p_semantic_param = semantic_param;
	}
    /* play welcome to indicate */
    if(aiengine_start(agn,p_semantic_param, uuid, _semantic_callback, usrdata) != 0)
    {
		ai_sem_flag.result = SEM_FAIL;
		ai_sem_flag.error = true;
        PERROR("aiengine start sem failed!\n");
        goto ULA_RELEASE;
    }

	/* init cound card */
/*	record_param param = {BIT, RATE, CHANEL_1, VOLUME};
	record_info = mozart_soundcard_init(param);
	if (!record_info) {
		PERROR("mozart_soundcard_init failed\n");
		goto ULA_RELEASE;
	}
	DEBUG("record format: bits: %d, rates: %u, channels: %u, volume: %d.\n",
			record_info->param.bits, record_info->param.rates,
			record_info->param.channels, record_info->param.volume);	//*/
    if(sound_device_init_near(VOLUME))
    {
		ai_sem_flag.result = SEM_FAIL;
		ai_sem_flag.error = true;
        PERROR("\"Sound device init failed!\"\n");
        goto ULA_RELEASE;
    }

    printf("Please Speak(播放李克勤的红日...)\n");
	buf_count =0;
    ai_sem_flag.state = SEM_STATUS_START;
//	printf("%d,%d,%d\n",ai_sem_flag.state,ai_sem_flag.set_end,ai_sem_flag.speak_end);
    while((ai_sem_flag.state == SEM_STATUS_START)      && !ai_sem_flag.set_end && !ai_sem_flag.speak_end) {

		ret = read(fd_dsp_rd, buf, RECORD_BUFSZ);
        if(ret < 0)
        {
			ai_sem_flag.result = SEM_FAIL;
			ai_sem_flag.error = true;
            PERROR("mozart_record failed\n");
            break;
        }
        ret = aiengine_feed(agn, buf, ret);
        if (ret < 0)
        {
			ai_sem_flag.result = SEM_FAIL;
			ai_sem_flag.error = true;
            PERROR("engine feed failed.\n");
            break;
        }
		if (++buf_count > aiengine_ini.asr.record_time){
		//	ai_sem_flag.result = SEM_EXIT;
		//	PERROR("Time out for record.\n");
			break;
		}
    }

    if(aiengine_stop(agn) != 0)
    {
		ai_sem_flag.result = SEM_FAIL;
        PERROR("aiengine stop failed!\n");
        goto SOUND_DEV_DISABLE;
    }
    printf("Waiting for the sem reuslt ...\n");
	#ifdef SYN_TOO_LONG
		gettimeofday(&t_sem_start,NULL);
	#endif
	loop = 0;
	//----------------- wake server return unturl timeout.
    while((ai_sem_flag.state == SEM_STATUS_START)       && !ai_sem_flag.set_end) {
        usleep(1000);
        if(++loop>aiengine_ini.asr.wait_time)
        {
            PERROR("No result found, Time out\n");
            ai_sem_flag.state = SEM_STATUS_FAIL;
			ai_sem_flag.result = SEM_NET_LOW;
			ai_sem_flag.error = true;
            break;
        }
    }
	if(ai_sem_flag.set_end){
		ai_sem_flag.result = SEM_EXIT;
		aiengine_cancel(agn);
	}
	else if(ai_sem_flag.state == SEM_STATUS_FAIL) {
		aiengine_cancel(agn);
	}
SOUND_DEV_DISABLE:
    sound_aec_disable();

SOUND_DEV_DEINIT:
    sound_device_release();

ULA_RELEASE:
	if (ai_sem_flag.error == true){
		ai_sem_flag.result = SEM_FAIL;
	}
OUT:
	free(new_semantic_param);
	new_semantic_param = NULL;
	ai_sem_flag.set_end = false;
	ai_sem_flag.state = SEM_STATUS_IDEL;
	ai_mutex_unlock();
    return ai_sem_flag.result;
}

int ai_add_sds_info_to_sem_param(void)
{

	cJSON *root = cJSON_Parse(semantic_param);
	if(!root) {
	    printf("get root faild !\n");
	    return -1;
	}

	cJSON *request = cJSON_GetObjectItem(root, "request");
	if(!request) {
	    printf("no request!\n");
	    return -1;
	}

	cJSON *sdsExpand = cJSON_GetObjectItem(request, "sdsExpand");
	if(!sdsExpand) {
	    printf("No sdsExpand !\n");
	    return -1;
	}

	cJSON *contextId = cJSON_GetObjectItem(sdsExpand, "contextId");
	if((!contextId)&&(recog.contextId)) {
		cJSON_AddStringToObject(sdsExpand,"contextId",recog.contextId);
	}

    cJSON *env = cJSON_GetObjectItem(request, "env");
    if((!env)&&(recog.env)){
        cJSON_AddStringToObject(request,"env",recog.env);
    }
	free(new_semantic_param);
	new_semantic_param = NULL;
	new_semantic_param = cJSON_Print(root);

	if(root){
	    cJSON_Delete(root);
    }
	    return 0;
}

