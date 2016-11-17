#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aiengine.h"
#include "cJSON.h"
#include "aiengine_app.h"
#include "ai_song_list.h"

static const char *semantic_text_param =
"\
{\
    \"coreProvideType\": \"cloud\",\
    \"vadEnable\": 0,\
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

static bool is_wait_callback = false;
static bool is_working = false;

extern struct aiengine *agn_text;

int _semantic_text_callback(const void *usrdata, const char *id, int type,
                                        const void *message, int size)

{
    int error =   0;
    cJSON *out = NULL;
    cJSON *result = NULL;
	cJSON *param = NULL;
	cJSON *error_j = NULL;
	cJSON *errId_j = NULL;
    out = cJSON_Parse((char*) message);
    if (!out)
    {
        error =   -1;
        PERROR("Error: out = null!...\n");
        goto exit_error;
    }

    result = cJSON_GetObjectItem(out, "result");
//    DEBUG("result: \n%s\n", cJSON_Print(result));
    if (result)
    {
		param = cJSON_GetObjectItem(result, "sds");
	    if (param){
	        if(ai_song_list_get_from_param(param) == -1){
	            PERROR("param_s error!\n");
		        error = -1;
		        goto exit_error;
	        }
	    }
		is_wait_callback = false;
    }//*/

	error_j = cJSON_GetObjectItem(out, "error");
    if (error_j)
    {
		is_wait_callback = false;
    }
exit_error:
    if (out)
    {
        cJSON_Delete(out);
    }
    return error;
}


int  _wait_end(void){
	int timeout = 0;

    while(is_wait_callback)
    {
        usleep(1000);
		timeout ++;
		if (timeout > 5000){	//	timeout    5 s
			PERROR("ERROR:   time out!\n");
			return -1;
		}
    }
	return 0;
}

void ai_cloud_sem_text_stop(void){
	int timeout = 0;
	if (is_working){
		is_wait_callback = false;
		DEBUG("ai_cloud_sem_text_stop stoping... !\n");
		while(is_working){
		  	usleep(1000);
			timeout ++;
			if (timeout > 5000){	//	timeout    5 s
				PERROR("ERROR:   time out!\n");
				return;
			}
		}
	}
}

/***************************************/
// Update new song from server
/***************************************/
int ai_cloud_sem_text(char *text){
	int error = 0;
    char uuid[64] = {0};
    char *_param = NULL;
    cJSON *param_js = NULL;
    cJSON *request_js = NULL;
    cJSON *syn_js = NULL;

	ai_mutex_lock();
	if (text == NULL){
		error = -1;
		goto exit_error;
	}
    if(!strcmp(text, ""))
    {
		error = -1;
		goto exit_error;
    }

    param_js = cJSON_Parse(semantic_text_param);
    if(param_js)
    {
        syn_js = cJSON_CreateString(text);
        request_js = cJSON_GetObjectItem(param_js, "request");
        cJSON_AddItemToObject(request_js, "refText", syn_js);
    }
    else
    {
		error = -1;
		goto exit_error;
    }
    _param = cJSON_PrintUnformatted(param_js);
	is_working = true;
	is_wait_callback = true;
    if(aiengine_start(agn_text,_param, uuid, _semantic_text_callback, NULL) != 0)
    {
        PERROR("ERROR:    failed!\n");
		aiengine_cancel(agn_text);
		error = -1;
        goto exit_error;
    }
	if (_wait_end()){
		aiengine_cancel(agn_text);
		PERROR("ERROR:    timeout!\n");
		error = -1;
        goto exit_error;
	}
exit_error:
	free(_param);
    if(param_js)
    {
        cJSON_Delete(param_js);
    }
	is_wait_callback = false;
	is_working = false;
	ai_mutex_unlock();
    return error;
}


