#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aiengine.h"
#include "ai_sound_dev.h"
#include "cJSON.h"
#include "aiengine_app.h"

#ifdef USE_ULA
#include "ula_doa.h"
#endif

enum ASR_STATUS
{
    ASR_SLEEP = 0,
    ASR_SUCCESS,
    ASR_FAIL
};

static int g_speak_end_flag = 0;
static int g_speak_stop_flag = 0;
static bool is_sem_error = false;
static int ai_cloudsem_working = 0;

static int recog_result_flag = ASR_SLEEP;
//extern char recog_buf[STR_BUFFER_SZ];
extern int fd_dsp_rd;
//extern char *MIC_CONFIG;

bool cloudsem_end_falg = false;

static char *sem_param = NULL;	//
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
extern int ai_errorPrint(int error_id);
#if 0//def USE_ULA
int phisDisplay(DOUBLE phis)
{
    printf("\nTheta[%.4f] Degree[%.4f]\n", (float)phis, (float)(phis*180/PI));
}
#endif
int _semantic_callback(void *usrdata, const char *id, int type,
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
	//    DEBUG("sem: size = %d\n%s\n",size,(char *)message);
  //  }
    out = cJSON_Parse((char*) message);
    if (!out)
    {
        return -1;
    }
    //vad status
    vad_status = cJSON_GetObjectItem(out, "vad_status");
  //  nbest = cJSON_GetObjectItem(out, "nbest");
    if (vad_status)
    {
        if(vad_status->valueint == 2)
        {
            // vad_status  0 - start , 1 -speaking, 2 - end
            g_speak_end_flag = 1;
            printf("*****************vad end**********************\n");
           // system("mplayer /usr/fs/usr/share/vr/searching.wav");
        }
        goto exit_error;
    }
	recordId = cJSON_GetObjectItem(out, "recordId");
	if(recordId){
		DEBUG("recordId: %s\n", recordId->valuestring);
		free(recog.recordId);
		recog.recordId = NULL;
		recog.recordId = strdup(recordId->valuestring);
		//	recog_result_flag = ASR_SUCCESS;
		//	recog.status = AIENGINE_STATUS_ASR_SUCCESS;
	}		//*/

    result = cJSON_GetObjectItem(out, "result");
    if (result)
    {
#ifdef SYN_TOO_LONG
    gettimeofday(&t_sem_end,NULL);
#endif
	//	free(sem_param);
	//	sem_param = NULL;
	//	sem_param = cJSON_PrintUnformatted(result);

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
			goto exit_error;
		}	//*/
		ai_sem_error_count =0;
		recog_result_flag = ASR_SUCCESS;
			//	recog.status = AIENGINE_STATUS_ASR_SUCCESS;
		//	}
    }

    error_j = cJSON_GetObjectItem(out, "error");
    if (error_j)
    {
	//	char *error_s = cJSON_Print(error_j);
	//	DEBUG("CLOUD SEM Error: \n%s\n", error_s);
	//	free(error_s);
 		errId_j = cJSON_GetObjectItem(out, "errId");
		if (errId_j){
			recog.error_id = errId_j->valueint;
		}
		recog.status = AIENGINE_STATUS_ERROR;
		recog_result_flag = ASR_FAIL;
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
	while(g_speak_stop_flag){
		timeout ++;
		usleep(10000);		//	100ms*50 = 5s
		if (++timeout > 5000){
			PERROR("ERROR: Stop cloud sem time out!\n");
			break;
		}
	}
	return 0;
}

void ai_cloud_sem_stop(void){
	DEBUG("=================> Stop cloud sem!\n");
	if (ai_cloudsem_working){
		cloudsem_end_falg = true;
		g_speak_stop_flag = 1;
		recog_result_flag = ASR_FAIL;
		ai_cloud_sem_stoping();
		cloudsem_end_falg = false;
	}
}

void ai_cloud_sem_free(void){
	free(recog.recordId);
	recog.recordId = NULL;
	free(sem_param);
	sem_param = NULL;
	free(new_semantic_param);
	new_semantic_param = NULL;
}

int ai_cloud_sem(struct aiengine *agn)
{
    char uuid[64] = {0};
	ai_cloudsem_working = 1;
	cloudsem_end_falg = false;
	is_sem_error = false;
	const void *usrdata  ;//= NULL;
#if 0 //def USE_ULA
	char *pcInData = NULL;
	char *pcOutData = NULL;
    ula_doa_t *pstDoa = NULL;
    ula_beamform_t *pstBeamform = NULL;
    ula_beamform_private_t stPriData;
#else
    char buf[RECORD_BUFSZ] = {0};
#endif
    int loop = 0;
    int ret = 0;
#if 0 //def USE_ULA
#ifdef ULA_DBG_SAVE_WAV
    char name[128] = {0};
    memset(name, 0, 128);
    sprintf(name, "%d.pcm", time());
    FILE *fpRaw = fopen(name, "ab+");
    memset(name, 0, 128);
    sprintf(name, "%d_bf.pcm", time());
    FILE *fpBF = fopen(name, "ab+");
#endif
#endif

    recog_result_flag = ASR_SLEEP;
    g_speak_end_flag = 0;
	g_speak_stop_flag = 0;
#if 0 //def USE_ULA
    /* private structure init */
    memset(&stPriData, 0, sizeof(stPriData));
    stPriData.chans = CHANEL_4;    /* actual channels */
    stPriData.fs = RATE;
    stPriData.ulMax = 8192*4*2; /* Max input data length */
    stPriData.ulTotalOutLen = stPriData.ulMax/2;    /* Max output data length */
    stPriData.lEndFlag = 0;
    memset(stPriData.acConfig, '\0', sizeof(stPriData.acConfig));
    strcpy(stPriData.acConfig,MIC_CONFIG);

    /* Doa and beamform init */
    ret = ULA_init(&stPriData, &pstDoa, &pstBeamform);
	if(ULA_RTN_FAIL == ret)
	{
		PERROR("Doa_beamformer module init fail.");
		goto OUT;
	}
    pstDoa->pGetTheta_callback = phisDisplay;
#endif
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
//	DEBUG("CLOUD SEM result: \n%s\n", p_semantic_param);
    if(aiengine_start(agn,p_semantic_param, uuid, _semantic_callback, usrdata) != 0)
    {
		ret = SEM_FAIL;
        PERROR("aiengine start sem failed!\n");
        goto ULA_RELEASE;
    }
#if 0//def USE_ULA
    /* alloc input data buffer */
    pcInData = calloc(1, RECORD_BUFSZ*4);
    if(NULL == pcInData)
    {
        goto ULA_RELEASE;
    }

    /* alloc output data buffer */
	pcOutData = calloc(1, stPriData.ulTotalOutLen);
    if(NULL == pcOutData)
    {
        goto ULA_RELEASE;
    }
    ULA_reset(pstDoa, pstBeamform);
#endif

#if 0 //def USE_ULA
    /* sound device init */
    ret = sound_device_init(VOLUME, CHANEL_4);
#else
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
	ret = sound_device_init_near(VOLUME);
#endif
    if(ret == -1)
    {
		ret = SEM_FAIL;
        PERROR("\"Sound device init failed!\"\n");
        goto ULA_RELEASE;
    }

/*    ret = sound_aec_enable();
    if(ret != 0)
    {
        PERROR("\"Sound device enable failed!\"\n");
        goto SOUND_DEV_DEINIT;
    }	//*/

    printf("Please Speak(播放李克勤的红日...)\n");
	int buf_count = 0;
    while(!g_speak_end_flag && !g_speak_stop_flag && !cloudsem_end_falg && !is_sem_error)
    {
		buf_count ++;
	//	printf("Time count = %d.\n",buf_count);
		if (buf_count > 230){
			ret = SEM_EXIT;
			PERROR("Time out for record.\n");
			break;
		}
#if 0 //def USE_ULA
        ret = read(fd_dsp_rd, pcInData, RECORD_BUFSZ*4);
        if(ret < 0)
        {
            PERROR("mozart_record failed\n");
            break;
        }
#ifdef ULA_DBG_SAVE_WAV
        fwrite(pcInData, ret, 1, fpRaw);
#endif

        ret = ULA_multi2monoPhisBF(pstDoa, pstBeamform, pcInData, ret, &stPriData, pcOutData);

#ifdef ULA_DBG_SAVE_WAV
        if(ret)
        {
            fwrite(pcOutData, ret, 1, fpBF);
        }
#endif
        ret = aiengine_feed(agn, pcOutData, ret);
        if (ret < 0)
        {
            PERROR("engine feed failed.\n");
            break;
        }
#else
		ret = read(fd_dsp_rd, buf, RECORD_BUFSZ);
        if(ret < 0)
        {
			ret = SEM_FAIL;
            PERROR("mozart_record failed\n");
            break;
        }
        ret = aiengine_feed(agn, buf, ret);
        if (ret < 0)
        {
			ret = SEM_FAIL;
            PERROR("engine feed failed.\n");
            break;
        }
#endif
    }

    if(aiengine_stop(agn) != 0)
    {
		ret = SEM_FAIL;
        PERROR("aiengine stop failed!\n");
        goto SOUND_DEV_DISABLE;
    }
    printf("Waiting for the sem reuslt ...\n");
	#ifdef SYN_TOO_LONG
		gettimeofday(&t_sem_start,NULL);
	#endif
    loop = 0;
    while(recog_result_flag == ASR_SLEEP && !g_speak_stop_flag)
    {
        usleep(1000);
        if(loop>15000)		//	15000 * 1ms = 15s
        {
            PERROR("No result found, Time out\n");
			aiengine_cancel(agn);
            recog_result_flag = ASR_FAIL;
			ret = SEM_NET_LOW;
            break;
        }
        loop++;
    }
	if(g_speak_stop_flag){
		ret = SEM_EXIT;
		aiengine_cancel(agn);
	}
	else if(recog_result_flag == ASR_FAIL) {
		if (ret == SEM_SUCCESS){
			ret = SEM_FAIL;
		}
		aiengine_cancel(agn);
	}

SOUND_DEV_DISABLE:
    sound_aec_disable();

SOUND_DEV_DEINIT:
    sound_device_release();

ULA_RELEASE:
#if 0//def USE_ULA

        free(pcOutData);
        pcOutData = NULL;

        free(pcInData);
        pcInDatROR NULL;

    ULA_beamformRelease(pstBeamform);
	ULA_doaRelease(pstDoa);
#endif
OUT:
#if 0// def USE_ULA
#ifdef ULA_DBG_SAVE_WAV
    fclose(fpBF);
    fclose(fpRaw);
#endif
#endif
	free(new_semantic_param);
	new_semantic_param = NULL;

	g_speak_stop_flag = 0;
	ai_cloudsem_working = 0;
    return ret;
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

