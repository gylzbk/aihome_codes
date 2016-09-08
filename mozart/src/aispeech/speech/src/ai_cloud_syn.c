#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cJSON.h"
#include "aiengine.h"

#include "aiengine_app.h"
////////////////////////////
#include   <sys/time.h>

///////////////////////////
int g_tts_end_flag = 0;
int is_tts_working;

static const char *cloud_syn_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"audio\": {\
        \"audioType\": \"mp3\",\
        \"channel\": 1,\
        \"sampleBytes\": 2,\
        \"sampleRate\": 16000,\
        \"compress\":\"raw\" \
    },\
    \"request\": {\
        \"coreType\": \"cn.sent.syn\",\
        \"res\": \"syn_chnsnt_zhilingf\",\
        \"speechRate\": 0.85,\
        \"speechVolume\": 50,\
        \"useStream\":1\
    }\
}";

#define cloud_sync_record "/tmp/cloud_sync_record.mp3"

char *fn = cloud_sync_record;

int _cloud_syn_callback(void *usrdata, const char *id, int type,
        const void *message, int size)
{
	if(type == 1)
    {
        printf(" Type is %d\n", type);
        g_tts_end_flag = 1;
        return 0;
    }else if(2 ==type)
	{
    	if (size == 0)
    	{
        	g_tts_end_flag = 1;
#ifdef SYN_TOO_LONG
		 	gettimeofday(&t_tts_end,NULL);
#endif
    	} else
    	{
			FILE *fout = NULL;
        	fout = fopen(fn, "a+");
        	if(fout)
        	{
            	fwrite(message, size, 1, fout);
            	fclose(fout);
        	}
    	}
	}
#if 0
    /* SYNC END*/

    if (size == 0)
    {
        g_tts_end_flag = 1;
#if 1 //def SYN_TOO_LONG
	 gettimeofday(&t_tts_end,NULL);
#endif

    }
    else
    {
		FILE *fout = NULL;
        fout = fopen(fn, "a+");
        if(fout)
        {
            fwrite(message, size, 1, fout);
            fclose(fout);
        }
    }
#endif
    return 0;
}

int  ai_tts_stoping(void){
	int timeout = 0;
    while(g_tts_end_flag == 0)
    {
        usleep(1000);
		timeout ++;
		if (timeout > 10000){	//	timeout    10 s
			PERROR("ERROR: tts time out!\n");
			break;
		}
    }
	return 0;
}

int  ai_tts_stop(void){
	DEBUG("=================> Stop tts!\n");
	if (is_tts_working){
		ai_tts_stoping();
	}
}

int ai_cloud_tts(struct aiengine *agn, char *SynTxt)
{
    char uuid[64] = {0};
    char *_param = NULL;
    cJSON *param_js = NULL;
    cJSON *request_js = NULL;
    cJSON *syn_js = NULL;
	int error = 0;
	is_tts_working = 1;
    g_tts_end_flag = 0;
	if (SynTxt == NULL){
		error = -1;
		goto exit_error;
	}
    if(!strcmp(SynTxt, ""))
    {
		error = -1;
		goto exit_error;
    }

    param_js = cJSON_Parse(cloud_syn_param);
    if(param_js)
    {
        syn_js = cJSON_CreateString(SynTxt);
        request_js = cJSON_GetObjectItem(param_js, "request");
        cJSON_AddItemToObject(request_js, "refText", syn_js);
    }
    else
    {
		error = -1;
		goto exit_error;
    }


    _param = cJSON_PrintUnformatted(param_js);

    DEBUG("=======tts syn=========\n");
    DEBUG("Syn: %s \n", SynTxt);
    FILE *fout =NULL;
	fout = fopen(fn, "w+");
    if (fout){
   	 	fclose(fout);
    }
	else{
		error = -1;
		goto exit_error;
	}
#ifdef SYN_TOO_LONG
	 gettimeofday(&t_tts_start,NULL);
#endif
    aiengine_start(agn, _param, uuid, _cloud_syn_callback, NULL);
    aiengine_stop(agn);
    /* waiting for the end of sync */
	ai_tts_stoping();

exit_error:
    free(_param);

    if(param_js)
    {
        cJSON_Delete(param_js);
    }
	is_tts_working = 0;
	return error;
}
