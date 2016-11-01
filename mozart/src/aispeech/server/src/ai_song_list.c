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
#include "ai_song_list.h"

#include "vr-speech_interface.h"

#include "aiengine_app.h"


#include "cJSON.h"

static const char *song_list_param =
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


ai_song_list_t ai_song_list;


/***************************************/
// Free
/***********************************/
void	_clean_list(void){
	int i;
	for (i=0;i<SONG_LIST_MAX;i++){
		free(ai_song_list.song[i].url);
		ai_song_list.song[i].url = NULL;
		free(ai_song_list.song[i].artist);
		ai_song_list.song[i].artist= NULL;
		free(ai_song_list.song[i].title);
		ai_song_list.song[i].title= NULL;
	}
	ai_song_list.push_num = 0;
	ai_song_list.song_num = 0;
	ai_song_list.is_success = false;
}

void _free_all(void){
	ai_song_list.type = SONG_TYPE_AUTO;
	free(ai_song_list.artist);
	ai_song_list.artist = NULL;
	ai_song_list.is_success = false;
	ai_song_list.is_getting = false;
	ai_song_list.song_num = 0;
	ai_song_list.push_num = 0;
}

/***************************************/
// Update new song from server
/***************************************/
int _renew_list(char *text){
	int error = 0;
    _clean_list();
	if (text == NULL){
		error = -1;
		goto exit_error;
	}

    if(!strcmp(text, "")) {
		error = -1;
		goto exit_error;
    }

	ai_cloud_sem_text(text);
exit_error:
    return error;
}

void _renew_server(void){
	int i = 0;
	if (ai_song_list.is_set_renew){
		ai_song_list.is_getting = true;
		if(recog.status == AIENGINE_STATUS_AEC){
			DEBUG("start renew list... \n");
			ai_song_list.is_set_renew = false;
			ai_song_list.renew_delay_s = 0;
			for(i=0; i<SONG_GET_ERROR_MAX; i++){
				if((ai_song_list.type == SONG_TYPE_AUTO)
				 ||(ai_song_list.artist == NULL)){
				//----------------------------- AUTO
				 	DEBUG("SONG_TYPE_AUTO... \n");
					if(_renew_list("我要听歌") == 0){
						break;
					}
				} else {
				//----------------------------- ARTIST
				 	DEBUG("SONG_TYPE_ARTIST... \n");
					if(_renew_list(ai_song_list.artist) == 0){
						break;
					}
				}
			}
		}
		ai_song_list.is_getting = false;
 	} else {
 	//	DEBUG("is_success = %d, renew_delay_s = %d , send_music = %d\n",
	//		ai_song_list.is_success,ai_song_list.renew_delay_s,ai_song_list.is_send_music);
		if (ai_song_list.is_success == false){
			if(ai_song_list.renew_delay_s++ > 30){
				ai_song_list.is_set_renew= true;
			}
		} else {
			if (ai_song_list.is_send_music == true){
				if(recog.status == AIENGINE_STATUS_AEC){
					if (aitalk_cloudplayer_is_playing() == false){
						usleep(10000);
						ai_song_list.is_send_music = false;
						ai_aitalk_send(aitalk_send_next_music(false));	//*/
						usleep(10000);
					}
				}
			}
		}
 	}
}


void *_run_thread(void *args)
{
	pthread_detach(pthread_self());
	int count = 0;
	ai_song_list.is_send_music = true;
	while (ai_song_list.is_working){
		while(ai_song_list.is_running){
			_renew_server();
			sleep(1);
		}
		sleep(1);
	}
}


/***************************************/
// APP
/***************************************/
int ai_song_list_get_from_param(cJSON *param){
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
		                ai_song_list.song_num = 0;
		                ai_song_list.push_num = 0;
	                    if (song_num>0){
		                    for(i=0;i<song_num;i++){
	                        dbdata_i = cJSON_GetArrayItem(dbdata, i);
	                        if (dbdata_i){
	                        //--------------------------------------------- get music url
	                            url = cJSON_GetObjectItem(dbdata_i, "url");
	                            if(url){
	                                if (url->valuestring){
				                        free(ai_song_list.song[ai_song_list.song_num].url);
				                        ai_song_list.song[ai_song_list.song_num].url = NULL;
				                        free(ai_song_list.song[ai_song_list.song_num].artist);
				                        ai_song_list.song[ai_song_list.song_num].artist = NULL;
				                        free(ai_song_list.song[ai_song_list.song_num].title);
				                        ai_song_list.song[ai_song_list.song_num].title = NULL;

				                        ai_song_list.song[ai_song_list.song_num].url = strdup(url->valuestring);
	                                    title = cJSON_GetObjectItem(dbdata_i, "title");
	                                    if (title){
	                                        if (title->valuestring){
				                                ai_song_list.song[ai_song_list.song_num].title = strdup(title->valuestring);
	                                        }
	                                    }
	                                    artist = cJSON_GetObjectItem(dbdata_i, "artist");
	                                    if(artist){
	                                        if (artist->valuestring){
				                                ai_song_list.song[ai_song_list.song_num].artist = strdup(artist->valuestring);
	                                        }
	                                    }
	                                }
	                            }else{
	                     	       DEBUG("no url error!\n");
	                  //              ret = -1;
	                  //              goto exit_error;
	                            }
			        //     	    DEBUG("artist= %s\n",ai_song_list.song[i].artist);
			       //             DEBUG("title= %s\n",ai_song_list.song[i].title);
			      //              DEBUG("url= %s\n",ai_song_list.song[i].url);
				                ai_song_list.song_num ++;
								ai_song_list.is_success = true;
				               	}else{
		                            DEBUG("i_data error!\n");
		                       	}
							}/*end for(i=0;i<song_num;i++)*/
	                   	}/*end if (song_num>0)*/
	                }/*end if(dbdata)*/
				}/*end data*/
			}/*end if(strcmp(domain->valuestring, "music") == 0)*/
		}/*eand if (domain->valuestring)*/
	}/*end if(domain)*/
	DEBUG("get music success      = %d , num = %d\n",ai_song_list.is_success,ai_song_list.song_num);
exit_error:
    return ret;
}


int ai_song_list_init(void){
	int i;
	pthread_t run_t;
	for (i=0;i<SONG_LIST_MAX;i++){
		ai_song_list.song[i].url= NULL;
		ai_song_list.song[i].artist = NULL;
		ai_song_list.song[i].title = NULL;
	}
	ai_song_list.type = SONG_TYPE_AUTO;
	ai_song_list.artist = NULL;
	ai_song_list.is_success = false;
	ai_song_list.is_getting = false;
	ai_song_list.song_num = 0;
	ai_song_list.push_num = 0;
	ai_song_list.is_working = true;
	ai_song_list.is_running = false;
	ai_song_list.is_set_renew = true;
	if (pthread_create(&run_t, NULL, _run_thread, NULL) != 0) {
		PERROR("Can't create ai_song_list_run_thread in : %s\n",strerror(errno));
		goto exit_error;
	}
	ai_song_list.is_init = true;
	return 0;

exit_error:
	PERROR("ai_song_list_init false! \n");
	ai_song_list.is_running = false;
	ai_song_list.is_working = false;
	ai_song_list.is_init = false;
	return -1;
}

void ai_song_list_exit(void){
	ai_song_list.is_working = false;
	ai_song_list.is_running = false;
	ai_cloud_sem_text_stop();
	_free_all();
}

int ai_song_list_set_enable(bool enable){
	if (enable){
		ai_song_list.is_running = true;
		ai_song_list.is_send_music = true;
	} else {
		ai_song_list.is_running = false;
		ai_song_list.is_send_music = false;
		ai_cloud_sem_text_stop();
	}
}

void ai_song_list_renew_auto(void){
	if(ai_song_list.artist){
		_clean_list();
		free(ai_song_list.artist);
		ai_song_list.artist = NULL;
	//	ai_song_list.set_send_music = true;
		ai_song_list.type = SONG_TYPE_AUTO;
		ai_song_list.is_success = false;
		ai_song_list.is_set_renew = true;
	}
}

void ai_song_list_renew_artist(char *artist){
	if (artist){
		_clean_list();
		//----------------------------- type
		free(ai_song_list.artist);
		ai_song_list.artist = NULL;
		ai_song_list.artist    = strdup(artist);
	//	ai_song_list.set_send_music = true;
		ai_song_list.type = SONG_TYPE_ARTIST;
		ai_song_list.is_success = false;
		ai_song_list.is_set_renew = true;
	}
}


/***************************************/
// push one song
/***************************************/
music_info *ai_song_list_push(void){
	int timeout = 0;
	if (ai_song_list.push_num >= ai_song_list.song_num){
		ai_song_list.is_set_renew = true;
		ai_song_list.is_success = false;
	}

//-------------------------------------- //
	while(!ai_song_list.is_success){
		usleep(1000);
		timeout ++;
		if (timeout > 10000){	//	10ms * 1000 = 10s
			PERROR("Error: time out! \n");
			usleep(10000);
			ai_aitalk_send(aitalk_send_error("error_net_slow_wait"));
			if (aitalk_cloudplayer_is_playing()){
				usleep(10000);
				ai_aitalk_send(aitalk_send_stop_music(false));	//*/
				ai_song_list.is_send_music = true;
				usleep(10000);
			}
			goto exit_error;
		}
	}


	if((ai_song_list.push_num >= ai_song_list.song_num)
	 &&(ai_song_list.push_num >= SONG_LIST_MAX)){
		PERROR("Error: push error ! \n");
		goto exit_error;
	}

	ai_song_list.is_send_music = false;
	return &ai_song_list.song[ai_song_list.push_num ++];
exit_error:
	return NULL;
}

