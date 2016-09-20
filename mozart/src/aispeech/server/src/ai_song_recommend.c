#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <netinet/in.h>
#include <linux/soundcard.h>

#include <pthread.h>
#include "ai_song_recommend.h"
#include "ai_server.h"

#include "vr-speech_interface.h"

#include "cloud_param.h"
#include "aiengine.h"
#include "aiengine_app.h"


#include "cJSON.h"
//#include "vr-speech_interface.h"
extern void ai_get_song_recommend(void);
extern bool aitalk_play_music;

static const char *song_recommand_param =
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


//char  *curl_ack = NULL;
//char  *get_new_song = NULL;
ai_song_recommend_t ai_song_update_list;
static bool is_callback_end = true;
static bool is_getting_song_recommend  = false;
extern struct aiengine *agn;
//char  *ai_song_recommend_sem_param;
/***************************************/
// Init
/***************************************/
void ai_song_recommend_init(void){
	int i;
	for (i=0;i<SONG_LIST_MAX;i++){
		ai_song_update_list.song[i].url= NULL;
		ai_song_update_list.song[i].artist = NULL;
		ai_song_update_list.song[i].title = NULL;
	}
	ai_song_update_list.type = SONG_RECOMMEND_TYPE_AUTO;
	ai_song_update_list.search_artist = NULL;
	ai_song_update_list.is_update_success = false;
	ai_song_update_list.is_update_getting = false;
	ai_song_update_list.song_number = 0;
	ai_song_update_list.geted_number = 0;

}


int  ai_song_recommend_stoping(void){
	int timeout = 0;

    while(is_callback_end == false)
    {
        usleep(1000);
		timeout ++;
		if (timeout > 10000){	//	timeout    10 s
			PERROR("ERROR: tts time out!\n");
			return -1;
		}
    }
	return 0;
}

/***************************************/
// Free
/***************************************/
void ai_song_recommend_free(void){
	int i;
	for (i=0;i<SONG_LIST_MAX;i++){
		free(ai_song_update_list.song[i].url);
		ai_song_update_list.song[i].url = NULL;
		free(ai_song_update_list.song[i].artist);
		ai_song_update_list.song[i].artist= NULL;
		free(ai_song_update_list.song[i].title);
		ai_song_update_list.song[i].title= NULL;
	}
	ai_song_update_list.geted_number = 0;
	ai_song_update_list.song_number = 0;
	ai_song_update_list.is_update_success = false;
	is_getting_song_recommend = false;
	is_callback_end = true;
}

void ai_song_recommend_free_all(void){
	ai_song_recommend_free();
	ai_song_update_list.type = SONG_RECOMMEND_TYPE_AUTO;
	free(ai_song_update_list.search_artist);
	ai_song_update_list.search_artist = NULL;
	ai_song_recommend_stoping();
	ai_song_update_list.is_update_success = false;
	ai_song_update_list.is_update_getting = false;
	ai_song_update_list.song_number = 0;
	ai_song_update_list.geted_number = 0;
}

int ai_song_recommend_get_from_param(cJSON *param){
    int ret = 0;
    int song_num=0;
	int i =0;
    cJSON *dbdata_i = NULL;
	cJSON *error = NULL;
	cJSON *type = NULL;
	cJSON *domain = NULL;
	cJSON *data = NULL;
	cJSON *dbdata = NULL;
	cJSON *url = NULL;
	cJSON *title = NULL;
	cJSON *artist = NULL;
// 	DEBUG("PASS %d\n",__LINE__);
    //  --------------------------------------------------- sds
    //  --------------------------------------------------- sds error
    error = cJSON_GetObjectItem(param, "error");
    if (error){
        type = cJSON_GetObjectItem(error, "type");
        if (type){
            PERROR("sds error = %s\n",type->valuestring);
        }
        ret = -1;
        goto exit_error;
    }
//	 	DEBUG("PASS %d\n",__LINE__);
    //  ------------------------------------------------- domain
    domain = cJSON_GetObjectItem(param, "domain");
    if(domain){
        if (domain->valuestring){
            if(strcmp(domain->valuestring, "music") == 0){
    //------------------------------------------------------- data
	            data = cJSON_GetObjectItem(param, "data");
	            if(data){
	                dbdata = cJSON_GetObjectItem(data, "dbdata");
	                if(dbdata){
	                    song_num = cJSON_GetArraySize(dbdata);
		                if (song_num > SONG_LIST_MAX){
			                song_num = SONG_LIST_MAX;
		                }
		                ai_song_update_list.song_number = 0;
		                ai_song_update_list.geted_number = 0;
	                    if (song_num>0){
		                    for(i=0;i<song_num;i++){
	                        dbdata_i = cJSON_GetArrayItem(dbdata, i);
	                        if (dbdata_i){
	                        //--------------------------------------------- get music url
	                            url = cJSON_GetObjectItem(dbdata_i, "url");
	                            if(url){
	                                if (url->valuestring){
				                        free(ai_song_update_list.song[ai_song_update_list.song_number].url);
				                        ai_song_update_list.song[ai_song_update_list.song_number].url = NULL;
				                        free(ai_song_update_list.song[ai_song_update_list.song_number].artist);
				                        ai_song_update_list.song[ai_song_update_list.song_number].artist = NULL;
				                        free(ai_song_update_list.song[ai_song_update_list.song_number].title);
				                        ai_song_update_list.song[ai_song_update_list.song_number].title = NULL;

				                        ai_song_update_list.song[ai_song_update_list.song_number].url = strdup(url->valuestring);
	                                    title = cJSON_GetObjectItem(dbdata_i, "title");
	                                    if (title){
	                                        if (title->valuestring){
				                                ai_song_update_list.song[ai_song_update_list.song_number].title = strdup(title->valuestring);
	                                        }
	                                    }
	                                    artist = cJSON_GetObjectItem(dbdata_i, "artist");
	                                    if(artist){
	                                        if (artist->valuestring){
				                                ai_song_update_list.song[ai_song_update_list.song_number].artist = strdup(artist->valuestring);
	                                        }
	                                    }
	                                }
	                            }else{
	                     	       DEBUG("no url error!\n");
	                  //              ret = -1;
	                  //              goto exit_error;
	                            }
			        //     	    DEBUG("artist= %s\n",ai_song_update_list.song[i].artist);
			      //             DEBUG("title= %s\n",ai_song_update_list.song[i].title);
			      //              DEBUG("url= %s\n",ai_song_update_list.song[i].url);
				                ai_song_update_list.song_number ++;
								ai_song_update_list.is_update_success = true;
				               	}else{
		                            DEBUG("i_data error!\n")
		                       	}
							}/*end for(i=0;i<song_num;i++)*/
	                   	}/*end if (song_num>0)*/
	                }/*end if(dbdata)*/
				}/*end data*/
			}/*end if(strcmp(domain->valuestring, "music") == 0)*/
		}/*eand if (domain->valuestring)*/
	}/*end if(domain)*/
	 	DEBUG("PASS %d\n",__LINE__);
exit_error:

 /*   if (param != NULL){
        cJSON_Delete(param);
        param = NULL;
    }   //*/

    return ret;
}



int ai_song_recommand_semantic_callback(void *usrdata, const char *id, int type,
                                        const void *message, int size)
{
    int error =   0;
    cJSON *out = NULL;
    cJSON *result = NULL;
	cJSON *param = NULL;
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
	        if(ai_song_recommend_get_from_param(param) == -1){
	            PERROR("param_s error!\n");
		        error = -1;
		        goto exit_error;
	        }
			is_callback_end = true;
	    }
    }//*/

exit_error:
    if (out)
    {
        cJSON_Delete(out);
    }

    return error;
}

/***************************************/
// Update new song from server
/***************************************/
int ai_song_recommend_update(char *text){
	int error = 0;
    char uuid[64] = {0};
    char *_param = NULL;
    cJSON *param_js = NULL;
    cJSON *request_js = NULL;
    cJSON *syn_js = NULL;

    ai_song_recommend_free();

	if (text == NULL){
		error = -1;
		goto exit_error;
	}
    if(!strcmp(text, ""))
    {
		error = -1;
		goto exit_error;
    }

    param_js = cJSON_Parse(song_recommand_param);
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

	is_callback_end = false;
    if(aiengine_start(agn,_param, uuid, ai_song_recommand_semantic_callback, NULL) != 0)
    {
        PERROR("ERROR: aiengine_start failed!\n");
		aiengine_cancel(agn);
		error = -1;
        goto exit_error;
    }
	if (ai_song_recommend_stoping()){
		error = -1;
        goto exit_error;
	}
exit_error:
	is_callback_end = true;
	free(_param);
    if(param_js)
    {
        cJSON_Delete(param_js);
    }
    return error;
}


/***************************************/
// Update new song from server
/***************************************/
music_info *ai_song_recommend_push(void){
//	music_info *song = NULL;
	int count = 0;
	//	get the list end    or not success
	if (ai_song_update_list.is_update_getting == true){
		PERROR("Error: Getting recommend song !... \n",count);
		return NULL;
	}

	ai_song_update_list.is_update_getting = true;
	if((ai_song_update_list.is_update_success == false)		//
		||(ai_song_update_list.geted_number >= ai_song_update_list.song_number)){
		for (count = 1;count<SONG_GET_ERROR_MAX;count++){
			ai_song_update_list.is_update_success = false;
			ai_song_update_list.geted_number = 0;
			ai_song_update_list.song_number =0;
			DEBUG("Get recommend song server time= %d!... \n",count);
			//------------------------------- SONG_RECOMMEND_TYPE_AUTO
			if((ai_song_update_list.type == SONG_RECOMMEND_TYPE_AUTO)
			 ||(ai_song_update_list.search_artist == NULL)){
			 	DEBUG("SONG_RECOMMEND_TYPE_AUTO... \n");
				if(ai_song_recommend_update("我要听歌") == 0){
					break;
				}
			}
			else{
			//----------------------------- SONG_RECOMMEND_TYPE_ARTIST
			 	DEBUG("SONG_RECOMMEND_TYPE_ARTIST... \n");
				if(ai_song_recommend_update(ai_song_update_list.search_artist) == 0){
					break;
				}
			}
		}
		if(ai_song_update_list.is_update_success == false){
			PERROR("Error: Get recommend song false!... \n");
			goto exit_error;
		}
	}
	if (ai_song_update_list.geted_number >= ai_song_update_list.song_number){
		ai_song_update_list.geted_number =0;
		ai_song_update_list.song_number = 0;
		count = 0;
	//	count = ai_song_update_list.song_number-1;
	}else{
//	song = ai_song_update_list.url[ai_song_update_list.geted_number];
		count = ai_song_update_list.geted_number;
	}
	ai_song_update_list.geted_number++;
	ai_song_update_list.is_update_getting = false;
	DEBUG("Get url = %d, %s... \n",count,ai_song_update_list.song[count]);
	return &ai_song_update_list.song[count];
exit_error:
	return NULL;
}


/***************************************/
// Update new song from server
/***************************************/
void ai_song_recommend_artist(char *artist){
	DEBUG("Get artist songs\n");
	if(artist == NULL){
		PERROR("artist = NULL! \n");
		return;
	}

	free(ai_song_update_list.search_artist);
	ai_song_update_list.search_artist = NULL;
	ai_song_update_list.search_artist    = strdup(artist);
	ai_song_update_list.type = SONG_RECOMMEND_TYPE_ARTIST;
	ai_song_update_list.is_update_success = false;
	ai_song_recommend_push();

	if (ai_song_update_list.is_update_success == true){
		DEBUG("Get song recommend successful, auto play music now.\n");
		#if AI_CONTROL_MOZART
		aitalk_play_music = true;
		ai_aitalk_send(aitalk_send_play_music(NULL));
		#endif
	}
}
#if 0
void *ai_song_recommend_auto_thr(void *args)
{
	int status= 0;
	int count = 0;
	is_getting_song_recommend = true;
	while (is_getting_song_recommend    ){
		sleep(1);
		DEBUG("Get song recommend,waiting...\n");
		if (ai_song_update_list.is_update_success == true){
			is_getting_song_recommend = false;
		}
		count ++;
		if (count > 20){
			is_getting_song_recommend = false;
			PERROR("Error: get song recommend timeout!...\n");
		}
	}

	if (ai_song_update_list.is_update_success == true){
		DEBUG("Get song recommend successful, auto play music now.\n");
		#if AI_CONTROL_MOZART
		aitalk_play_music = true;
		ai_aitalk_send(aitalk_send_play_music(NULL));
		#endif
	}
	pthread_exit(&status);
}
#endif

int ai_song_recommend_auto(void){
	music_info *music = NULL;
	ai_song_recommend_push();
	if (ai_song_update_list.is_update_success == true){
		DEBUG("Get song recommend successful, auto play music now.\n");
		#if AI_CONTROL_MOZART
		aitalk_play_music = true;
		music = &ai_song_update_list.song[0];
		if (music){
			if (music->url != NULL){
				ai_music_list_add_music(music);
			}
			else{
				return 0;
			}
		}
		ai_aitalk_send(aitalk_send_play_url(music));
		ai_song_update_list.geted_number++;
		#endif
	}
	return 0;
}

