#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/soundcard.h>

#include "ai_server.h"
#include "ai_slot.h"
#include "ai_error.h"
#include "cJSON.h"

//#define TIMED_UPLOAD (total_song)
#define get_nonce(nonce,total) \
do{ \
	struct timeval tpstart;\
	gettimeofday(&tpstart,NULL); \
	srand(tpstart.tv_usec); \
	nonce = 1+(int)(total*1.0*rand()/(RAND_MAX*1.0)); \
	}while(0)

const char *aiengineStatus[AIENGINE_STATUS_MAX]
			= {"INIT","AEC","SEM","PROCESS","ERROR","STOP","EXIT"};//	NULL,volume;
const char *aiSlotDomain[RECOG_DOMAIN_MAX]
			= {"null","chat","music","weather","calendar","command","netfm","reminder","stock","poetry","movie"};
const char *aiSlotObject[SDS_OBJECT_MAX]
			= {"null","volume","music"};
const char *aiSlotState[SDS_STATE_MAX]
			= {"null","do","query","offernone","offer","interact","exit"};
const char *aiSlotCommand[SDS_COMMAND_MAX]
			= {"null","音量","操作"};//	NULL,volume;

#if 1

int ai_slot_recog_free(vr_info *recog){
	recog->state =	   SDS_STATE_NULL;
	recog->domain = RECOG_DOMAIN_NULL;
	recog->is_control_play_music = false;
	free(recog->input);
	recog->input = NULL;
	free(recog->output);
	recog->output = NULL;
	free(recog->contextId);
	recog->contextId = NULL;
	free(recog->volume);
	recog->volume = NULL;
	free(recog->object);
	recog->object = NULL;
	free(recog->operation);
	recog->operation = NULL;
	free(recog->scene);
	recog->scene = NULL;
	free(recog->device);
	recog->device = NULL;
	free(recog->location);
	recog->location = NULL;

	free(recog->search_artist);
	recog->search_artist= NULL;
	free(recog->search_title);
	recog->search_title= NULL;

	recog->music_num =0;
	free(recog->music.artist);
	recog->music.artist = NULL;
	free(recog->music.title);
	recog->music.title = NULL;
	free(recog->music.url);
	recog->music.url = NULL;

	free(recog->weather.temperature);
	recog->weather.temperature = NULL;
	free(recog->weather.weather);
	recog->weather.weather= NULL;
	free(recog->weather.wind);
	recog->weather.wind= NULL;
	free(recog->weather.area);
	recog->weather.area= NULL;

	free(recog->movie.name);
	recog->movie.name = NULL;
	free(recog->movie.sequence);
	recog->movie.sequence= NULL;
	free(recog->movie.director);
	recog->movie.director = NULL;
	free(recog->movie.player);
	recog->movie.player= NULL;
	free(recog->movie.type);
	recog->movie.type= NULL;
	free(recog->movie.area);
	recog->movie.area= NULL;

	free(recog->date);
	recog->date = NULL;
	free(recog->time);
	recog->time = NULL;
	free(recog->event);
	recog->event = NULL;
	free(recog->env);
	recog->env = NULL;
}


int ai_slot_resolve(vr_info *recog,cJSON *sem_json){

	int ret = 0;
	cJSON *dbdata_i = NULL;
	cJSON *param = NULL;
//	cJSON *sem_json = NULL;
	cJSON *input = NULL;
	cJSON *semantics = NULL;
	cJSON *request = NULL;
	cJSON *domain = NULL;
	cJSON *state = NULL;
	cJSON *param_json = NULL;
	cJSON *operation = NULL;
	cJSON *scene = NULL;
	cJSON *device = NULL;
	cJSON *location = NULL;
	cJSON *object = NULL;
	cJSON *search_artist = NULL;
	cJSON *search_title = NULL;
	cJSON *volume = NULL;
	cJSON *name	  = NULL;
	cJSON *sequence = NULL;
	cJSON *player = NULL;
	cJSON *director = NULL;
	cJSON *type = NULL;
	cJSON *area = NULL;
	cJSON *error = NULL;
	cJSON *output = NULL;
	cJSON *contextId = NULL;
	cJSON *nlu = NULL;
	cJSON *date = NULL;
	cJSON *time = NULL;
	cJSON *event = NULL;
	cJSON *data = NULL;
	cJSON *dbdata = NULL;
	cJSON *today = NULL;
	cJSON *weather = NULL;
	cJSON *wind = NULL;
	cJSON *temperature = NULL;
	cJSON *url = NULL;
	cJSON *title = NULL;
	cJSON *artist = NULL;
	cJSON *url_64 = NULL;
	cJSON *url_32 = NULL;
/*	if (sem_param == NULL){
		ret = -1;
		PERROR("sem_param = NULL\n");
		goto exit_error;
	}
	sem_json = cJSON_Parse((char*) sem_param);
	if (sem_json == NULL){
		ret = -1;
		PERROR("sem_json = NULL\n");
		goto exit_error;
	}	//*/

	ai_slot_recog_free(recog);
#if 1
	char* c_json = cJSON_Print(sem_json);
	printf("\n%s\n",c_json);
	free(c_json);
#endif
	//	--------------------------------------------------- input
	input = cJSON_GetObjectItem(sem_json, "input");
	if (input){
		if(input->valuestring){
			free(recog->input);
			recog->input = strdup(input->valuestring);
		}
	}
	//	------------------------------------------------- semantics
	semantics = cJSON_GetObjectItem(sem_json, "semantics");
	if(semantics){
		//	------------------------------------------------- request
		request = cJSON_GetObjectItem(semantics, "request");
		if(request){
				domain = cJSON_GetObjectItem(request, "domain");
				if(domain){
					if (domain->valuestring){
						if(strcmp(domain->valuestring, "电影搜索") == 0){
							recog->domain = RECOG_DOMAIN_MOVIE;
						}
					}
				}
				//	------------------------------------------------- param
				param_json = cJSON_GetObjectItem(request, "param");
				if(param_json){
				//	------------------------------------------------- operation
					operation = cJSON_GetObjectItem(param_json, "操作");
					if(operation){
						if(operation->valuestring){
							free(recog->operation);
							recog->operation = strdup(operation->valuestring);
						}
					}
				//	------------------------------------------------- scene
					scene = cJSON_GetObjectItem(param_json, "模式");
					if(scene){
						if(scene->valuestring){
							free(recog->scene);
							recog->scene = strdup(scene->valuestring);
						}
					}
				//	------------------------------------------------- device
					device = cJSON_GetObjectItem(param_json, "设备名称");
					if(device){
						if(device->valuestring){
							free(recog->device);
							recog->device = strdup(device->valuestring);
						}
					}
				//	------------------------------------------------- location
					location = cJSON_GetObjectItem(param_json, "位置");
					if(location){
						if(location->valuestring){
							free(recog->location);
							recog->location = strdup(location->valuestring);
						}
					}
				//	------------------------------------------------- object
					object = cJSON_GetObjectItem(param_json, "对象");
					if(object){
						if(object->valuestring){
							free(recog->object);
							recog->object = strdup(object->valuestring);
						}
					}	//*/
				//	------------------------------------------------- volume
					volume = cJSON_GetObjectItem(param_json, "音量");
					if(volume){
						if(volume->valuestring){
							free(recog->volume);
							recog->volume = strdup(volume->valuestring);
						}
					}
				//	------------------------------------------------- search_artist
					search_artist = cJSON_GetObjectItem(param_json, "歌手名");
					if(search_artist){
						if(search_artist->valuestring){
							free(recog->search_artist);
							recog->search_artist = strdup(search_artist->valuestring);
						}
					}
				//	------------------------------------------------- search_title
					search_title = cJSON_GetObjectItem(param_json, "歌曲名");
					if(search_title){
						if(search_title->valuestring){
							free(recog->search_title);
							recog->search_title = strdup(search_title->valuestring);
						}
					}

					if (recog->domain == RECOG_DOMAIN_MOVIE){
						name = cJSON_GetObjectItem(param_json, "片名");
						if(name){
							if(name->valuestring){
								free(recog->movie.name);
								recog->movie.name = strdup(name->valuestring);
							}
						}
						sequence = cJSON_GetObjectItem(param_json, "序列号");
						if(sequence){
							if(sequence->valuestring){
								free(recog->movie.sequence);
								recog->movie.sequence = strdup(sequence->valuestring);
							}
						}
						player = cJSON_GetObjectItem(param_json, "主演");
						if(player){
							if(player->valuestring){
								free(recog->movie.player);
								recog->movie.player = strdup(player->valuestring);
							}
						}
						else{
							player = cJSON_GetObjectItem(param_json, "电影人");
							if(player){
								if(player->valuestring){
									free(recog->movie.player);
									recog->movie.player = strdup(player->valuestring);
								}
							}
						}
						director = cJSON_GetObjectItem(param_json, "导演");
						if(director){
							if(director->valuestring){
								free(recog->movie.director);
								recog->movie.director = strdup(director->valuestring);
							}
						}
						type = cJSON_GetObjectItem(param_json, "类型");
						if(type){
							if(type->valuestring){
								free(recog->movie.type);
								recog->movie.type = strdup(type->valuestring);
							}
						}
						area = cJSON_GetObjectItem(param_json, "国家地区");
						if(area){
							if(area->valuestring){
								free(recog->movie.area);
								recog->movie.area = strdup(area->valuestring);
							}
						}
						goto exit_slot;
					}

				}
		    }
	    }

	//	--------------------------------------------------- sds
	param = cJSON_GetObjectItem(sem_json, "sds");
	if (param == NULL){
		PERROR("sds param = NULL\n");
		goto exit_error;
	}
	//	--------------------------------------------------- sds error
	error = cJSON_GetObjectItem(param, "error");
	if (error){
		type = cJSON_GetObjectItem(error, "type");
		if (type){
		//	PERROR("sds error = %s\n",type->valuestring);
			recog->error_type= AI_ERROR_UNKNOW;
			if (type->valuestring){
				if(strcmp(type->valuestring, "INVALID DOMAIN") == 0){
					recog->error_type= AI_ERROR_INVALID_DOMAIN;
				}
			}
		}
		goto exit_error;
	}

	//	------------------------------------------------- domain
	domain = cJSON_GetObjectItem(param, "domain");
	if(domain){
		if (domain->valuestring){
			free(recog->env);
			recog->env = NULL;
			if(strcmp(domain->valuestring, "music") == 0){
				recog->domain = RECOG_DOMAIN_MUSIC;
				recog->env = strdup("dlg_domain=\"音乐\";");
				if ((recog->search_title==NULL)
				  &&(recog->search_artist)){
					goto exit_slot;
				}
			}	//*/
			else if(strcmp(domain->valuestring, "chat") == 0){
				recog->domain = RECOG_DOMAIN_CHAT;
				recog->env = strdup("dlg_domain=\"聊天\";");
			}
			else if(strcmp(domain->valuestring, "calendar") == 0){
				recog->domain = RECOG_DOMAIN_CALENDAR;
				recog->env = strdup("dlg_domain=\"日历\";");
			}
			else if(strcmp(domain->valuestring, "weather") == 0){
				recog->domain = RECOG_DOMAIN_WEATHER;
				recog->env = strdup("dlg_domain=\"天气\";");
			}
			else if(strcmp(domain->valuestring, "netfm") == 0){
				recog->domain = RECOG_DOMAIN_NETFM;
				recog->env = strdup("dlg_domain=\"电台\";");
			}
			else if(strcmp(domain->valuestring, "command") == 0){
				recog->domain = RECOG_DOMAIN_COMMAND;
				recog->env = strdup("dlg_domain=\"中控\";");
			}
			else if(strcmp(domain->valuestring, "reminder") == 0){
				recog->domain = RECOG_DOMAIN_REMINDER;
				recog->env = strdup("dlg_domain=\"提醒\";");
			}
			else if(strcmp(domain->valuestring, "stock") == 0){
				recog->domain = RECOG_DOMAIN_STOCK;
				recog->env = strdup("dlg_domain=\"股票\";");
			}
			else if(strcmp(domain->valuestring, "poetry") == 0){
				recog->domain = RECOG_DOMAIN_POETRY;
				recog->env = strdup("dlg_domain=\"古诗\";");
			}

		}
	}

	if(recog->domain == RECOG_DOMAIN_COMMAND){

		goto exit_slot;
	}

	//-------------------------------------------------------         		state
	state = cJSON_GetObjectItem(param, "state");
	if(state){
	//	DEBUG("state : %s\n", state->valuestring);
		if (state->valuestring){
			if(strcmp(state->valuestring, "do") == 0){
				recog->state = SDS_STATE_DO;
			}
			else if(strcmp(state->valuestring, "query") == 0){
				recog->state = SDS_STATE_QUERY;
			}
			else if(strcmp(state->valuestring, "interact") == 0){
				recog->state = SDS_STATE_INTERACT;
			}
			else if(strcmp(state->valuestring, "offer") == 0){
				recog->state = SDS_STATE_OFFER;
			}
			else if(strcmp(state->valuestring, "offernone") == 0){
				recog->state = SDS_STATE_OFFERNONE;
			}
			else if(strcmp(state->valuestring, "exit") == 0){
				recog->state = SDS_STATE_EXIT;
			}
			else{
				recog->state = SDS_STATE_NULL;
			}
		}
	}	//*/
	//-------------------------------------------------------  				output
	output = cJSON_GetObjectItem(param, "output");
	if(output){
		if (output->valuestring){
			free(recog->output);
			recog->output = strdup(output->valuestring);
		}
	}	//*/
	//-------------------------------------------------------  				contextId
	contextId = cJSON_GetObjectItem(param, "contextId");
	if(contextId){
		if (contextId->valuestring){
			free(recog->contextId);
			recog->contextId = strdup(contextId->valuestring);
		}
	}	//*/
	//-------------------------------------------------------  				nlu
	nlu = cJSON_GetObjectItem(param, "nlu");
	if(nlu){
	    date = cJSON_GetObjectItem(nlu, "date");
	    time = cJSON_GetObjectItem(nlu, "time");
        if(date&&time){
		    if (date->valuestring&&time->valuestring){
				free(recog->date);
			    recog->date = strdup(date->valuestring);
				free(recog->time);
			    recog->time = strdup(time->valuestring);
	            event = cJSON_GetObjectItem(nlu, "event");
                if(event){
		            if (event->valuestring){
						free(recog->event);
			            recog->event = strdup(event->valuestring);
                    }
                }
            }
        }
	}	//*/

	//-------------------------------------------------------  				data
	data = cJSON_GetObjectItem(param, "data");
	if(data){
		dbdata = cJSON_GetObjectItem(data, "dbdata");
		if(dbdata){
			DEBUG("PASS\n");
			if(recog->domain == RECOG_DOMAIN_WEATHER){
				DEBUG("PASS\n");
				int number = cJSON_GetArraySize(dbdata);
				DEBUG("number = %d\n",number);
				if (number ==1){
					dbdata_i = cJSON_GetArrayItem(dbdata, 0);
					area = cJSON_GetObjectItem(dbdata_i, "area");
					if(area){
						if (area->valuestring){
							free(recog->weather.area);
							recog->weather.area = strdup(area->valuestring);
						}
					}
					today = cJSON_GetObjectItem(dbdata_i, "today");
					if (today)
					{
						weather = cJSON_GetObjectItem(today, "weather");
						if(weather){
							if (weather->valuestring){
								free(recog->weather.weather);
								recog->weather.weather = strdup(weather->valuestring);
							}
						}
						wind = cJSON_GetObjectItem(today, "wind");
						if(wind){
							if (wind->valuestring){
								free(recog->weather.wind);
								recog->weather.wind = strdup(wind->valuestring);
							}
						}
						temperature = cJSON_GetObjectItem(today, "temperature");
						if(temperature){
							if (temperature->valuestring){
								free(recog->weather.temperature);
								recog->weather.temperature = strdup(temperature->valuestring);
							}
						}
					}
				}
			}
			else if((recog->domain == RECOG_DOMAIN_MUSIC)
			||(recog->domain == RECOG_DOMAIN_NETFM)
			||(recog->domain == RECOG_DOMAIN_CHAT)){
				recog->music_num = cJSON_GetArraySize(dbdata);
				int total_song=recog->music_num;
				int song_num = 1;
			//	get_nonce(song_num,total_song);
				DEBUG("total_song = %d,  play_num = %d\n",total_song,song_num);
				if ((song_num>0)&&(song_num<=total_song)){
					dbdata_i = cJSON_GetArrayItem(dbdata, song_num-1);

					if (dbdata_i){
						switch(recog->domain){
						//--------------------------------------------- get music url
						case RECOG_DOMAIN_MUSIC:
							url = cJSON_GetObjectItem(dbdata_i, "url");
							if(url){
								if (url->valuestring){
									free(recog->music.url);
									recog->music.url = strdup(url->valuestring);
								}
								title = cJSON_GetObjectItem(dbdata_i, "title");
								if (title){
									if (title->valuestring){
										free(recog->music.title);
										recog->music.title = strdup(title->valuestring);
									}
								}
								artist = cJSON_GetObjectItem(dbdata_i, "artist");
								if(artist){
									if (artist->valuestring){
										free(recog->music.artist);
										recog->music.artist = strdup(artist->valuestring);
									}
								}
							}
							break;
						//--------------------------------------------- get netfm url
						case RECOG_DOMAIN_NETFM:
							url_64 = cJSON_GetObjectItem(dbdata_i, "playUrl64");
							if(url_64){
								if (url_64->valuestring){
									free(recog->music.url);
									recog->music.url = strdup(url_64->valuestring);
									title = cJSON_GetObjectItem(dbdata_i, "track");
									if (title){
										if (title->valuestring){
											free(recog->music.title);
											recog->music.title = strdup(title->valuestring);
										}
									}
									break;
								}
							}
							url_32 = cJSON_GetObjectItem(dbdata_i, "playUrl32");
							if(url_32){
								if(url_32->valuestring){
									free(recog->music.url);
									recog->music.url = strdup(url_32->valuestring);
									title = cJSON_GetObjectItem(dbdata_i, "track");
									if (title){
										if(title->valuestring){
											free(recog->music.title);
											recog->music.title = strdup(title->valuestring);
										}
									}
								}
							}
							break;
						//--------------------------------------------- get char url
						case RECOG_DOMAIN_CHAT:
							url = cJSON_GetObjectItem(dbdata_i, "url");
							if(url){
								if (url->valuestring){
									free(recog->music.url);
									recog->music.url = strdup(url->valuestring);
								}
							}
							break;
						}
					}
					else{
						PERROR("dbdata_i error!\n");
					}
				}
				else if(recog->domain != RECOG_DOMAIN_REMINDER){
					//DEBUG("song_num = %d ,error!\n",song_num);
				}
			}

		}
	}
	goto exit_slot;

exit_error:
	ai_slot_recog_free(recog);
exit_slot:
	DEBUG("input : %s\n", recog->input);
	DEBUG("output : %s\n", recog->output);
	DEBUG("contextId : %s\n", recog->contextId);
	DEBUG("domain : %s\n", aiSlotDomain[recog->domain]);
	DEBUG("state : %s\n", aiSlotState[recog->state]);
	DEBUG("operation : %s\n", recog->operation);
	DEBUG("scene : %s\n", recog->scene);
	DEBUG("device : %s\n", recog->device);
	DEBUG("location : %s\n", recog->location);
	DEBUG("object : %s\n", recog->object);
	DEBUG("date : %s\n", recog->date);
	DEBUG("time : %s\n", recog->time);
	DEBUG("event : %s\n", recog->event);

	DEBUG("search_artist : %s\n", recog->search_artist);//*/
	DEBUG("search_title : %s\n", recog->search_title);//*/
	DEBUG("url_number : %d\n", recog->music_num);
	DEBUG("url : %s\n", recog->music.url);
	DEBUG("artist : %s\n", recog->music.artist);
	DEBUG("title : %s\n", recog->music.title);//*/
	DEBUG("area : %s\n", recog->weather.area);//*/
	DEBUG("weather : %s\n", recog->weather.weather);//*/
	DEBUG("temperature : %s\n", recog->weather.temperature);//*/
	DEBUG("wind : %s\n", recog->weather.wind);//*/
	DEBUG("movie name : %s\n", recog->movie.name);//*/
	DEBUG("movie sequence : %s\n", recog->movie.sequence);//*/
	DEBUG("movie director : %s\n", recog->movie.director);//*/
	DEBUG("movie player : %s\n", recog->movie.player);//*/
	DEBUG("movie type : %s\n", recog->movie.type);//*/
	DEBUG("movie area : %s\n", recog->movie.area);//*/

/*	if (param != NULL){
		cJSON_Delete(param);
		param = NULL;
	}	//*/
	if (recog->domain == RECOG_DOMAIN_NULL)
		ret = -1;

	if (ret)
		recog->status = AIENGINE_STATUS_ERROR;

	return ret;
}
#else

int ai_slot_recog_free(vr_info *recog){
	recog->state =	   SDS_STATE_NULL;
	recog->domain = RECOG_DOMAIN_NULL;
	//return 0;
	//free(recog->input);
	recog->input = NULL;
	//free(recog->output);
	recog->output = NULL;
	//free(recog->contextId);
	recog->contextId = NULL;
	//free(recog->volume);
	recog->volume = NULL;
	//free(recog->object);
	recog->object = NULL;
	//free(recog->operation);
	recog->operation = NULL;
	//free(recog->device);
	recog->device = NULL;
	//free(recog->location);
	recog->location = NULL;

	//free(recog->search_artist);
	recog->search_artist= NULL;
	//free(recog->search_title);
	recog->search_title= NULL;

	recog->music_num =0;
	//free(recog->music.artist);
	recog->music.artist = NULL;
	//free(recog->music.title);
	recog->music.title = NULL;
	//free(recog->music.url);
	recog->music.url = NULL;

	//free(recog->weather.temperature);
	recog->weather.temperature = NULL;
	//free(recog->weather.weather);
	recog->weather.weather= NULL;
	//free(recog->weather.wind);
	recog->weather.wind= NULL;


	//free(recog->movie.name);
	recog->movie.name = NULL;
	//free(recog->movie.sequence);
	recog->movie.sequence= NULL;
	//free(recog->movie.director);
	recog->movie.director = NULL;
	//free(recog->movie.player);
	recog->movie.player= NULL;
	//free(recog->movie.type);
	recog->movie.type= NULL;
	//free(recog->movie.area);
	recog->movie.area= NULL;

	//free(recog->date);
	recog->date = NULL;
	//free(recog->time);
	recog->time = NULL;
	//free(recog->event);
	recog->event = NULL;
	//free(recog->env);
	recog->env = NULL;
	return 0;
}

int ai_slot_resolve(vr_info *recog,char *sem_param){

	int ret = 0;
	cJSON *dbdata_i = NULL;
	cJSON *param = NULL;
	cJSON *sem_json = NULL;
	cJSON *input = NULL;
	cJSON *semantics = NULL;
	cJSON *request = NULL;
	cJSON *domain = NULL;
	cJSON *state = NULL;
	cJSON *param_json = NULL;
	cJSON *operation = NULL;
	cJSON *device = NULL;
	cJSON *location = NULL;
	cJSON *object = NULL;
	cJSON *search_artist = NULL;
	cJSON *search_title = NULL;
	cJSON *volume = NULL;
	cJSON *name	  = NULL;
	cJSON *sequence = NULL;
	cJSON *player = NULL;
	cJSON *director = NULL;
	cJSON *type = NULL;
	cJSON *area = NULL;
	cJSON *error = NULL;
	cJSON *output = NULL;
	cJSON *contextId = NULL;
	cJSON *nlu = NULL;
	cJSON *date = NULL;
	cJSON *time = NULL;
	cJSON *event = NULL;
	cJSON *data = NULL;
	cJSON *dbdata = NULL;
	cJSON *today = NULL;
	cJSON *weather = NULL;
	cJSON *wind = NULL;
	cJSON *temperature = NULL;
	cJSON *url = NULL;
	cJSON *title = NULL;
	cJSON *artist = NULL;
	cJSON *url_64 = NULL;
	cJSON *url_32 = NULL;
	DEBUG("SEM = \n%s\n",sem_param);
	if (sem_param == NULL){
		ret = -1;
		PERROR("sem_param = NULL\n");
		goto exit_error;
	}
	sem_json = cJSON_Parse((char*) sem_param);
	if (sem_json == NULL){
		ret = -1;
		PERROR("sem_json = NULL\n");
		goto exit_error;
	}

	ai_slot_recog_free(recog);
	//	--------------------------------------------------- input
	input = cJSON_GetObjectItem(sem_json, "input");
	if (input){
		recog->input = input->valuestring;
	}
	//	------------------------------------------------- semantics
	semantics = cJSON_GetObjectItem(sem_json, "semantics");
	if(semantics){
		//	------------------------------------------------- request
		request = cJSON_GetObjectItem(semantics, "request");
		if(request){
				domain = cJSON_GetObjectItem(request, "domain");
				if(domain){
					if (domain->valuestring){
						if(strcmp(domain->valuestring, "电影搜索") == 0){
							recog->domain = RECOG_DOMAIN_MOVIE;
						}
					}
				}
				//	------------------------------------------------- param
				param_json = cJSON_GetObjectItem(request, "param");
				if(param_json){
				//	------------------------------------------------- operation
					operation = cJSON_GetObjectItem(param_json, "操作");
					if(operation){
						recog->operation = 	operation->valuestring;
					}
				//	------------------------------------------------- device
					device = cJSON_GetObjectItem(param_json, "设备名称");
					if(device){
						recog->device = device->valuestring;
					}
				//	------------------------------------------------- location
					location = cJSON_GetObjectItem(param_json, "位置");
					if(location){
						recog->location = location->valuestring;
					}
				//	------------------------------------------------- object
					object = cJSON_GetObjectItem(param_json, "对象");
					if(object){
						recog->object = object->valuestring;
					}
				//	------------------------------------------------- volume
					volume = cJSON_GetObjectItem(param_json, "音量");
					if(volume){
						recog->volume = volume->valuestring;
					}
				//	------------------------------------------------- search_artist
					search_artist = cJSON_GetObjectItem(param_json, "歌手名");
					if(search_artist){
						recog->search_artist = search_artist->valuestring;
					}
				//	------------------------------------------------- search_title
					search_title = cJSON_GetObjectItem(param_json, "歌曲名");
					if(search_title){
						recog->search_title = search_title->valuestring;
					}

					if (recog->domain == RECOG_DOMAIN_MOVIE){
						name = cJSON_GetObjectItem(param_json, "片名");
						if(name){
							recog->movie.name = name->valuestring;
						}
						sequence = cJSON_GetObjectItem(param_json, "序列号");
						if(sequence){
							recog->movie.sequence = sequence->valuestring;
						}
						player = cJSON_GetObjectItem(param_json, "主演");
						if(player){
							recog->movie.player = player->valuestring;
						}
						else{
							player = cJSON_GetObjectItem(param_json, "电影人");
							if(player){
								recog->movie.player = player->valuestring;
							}
						}
						director = cJSON_GetObjectItem(param_json, "导演");
						if(director){
							recog->movie.director = director->valuestring;
						}
						type = cJSON_GetObjectItem(param_json, "类型");
						if(type){
							recog->movie.type = type->valuestring;
						}
						area = cJSON_GetObjectItem(param_json, "国家地区");
						if(area){
							recog->movie.area = area->valuestring;
						}
						goto exit_slot;
					}
				}
		    }
	    }

	//	--------------------------------------------------- sds
	param = cJSON_GetObjectItem(sem_json, "sds");
	if (param == NULL){
		PERROR("sds param = NULL\n");
		goto exit_error;
	}
	//	--------------------------------------------------- sds error
	error = cJSON_GetObjectItem(param, "error");
	if (error){
		type = cJSON_GetObjectItem(error, "type");
		if (type){
			PERROR("sds error = %s\n",type->valuestring);
			recog->error_type= AI_ERROR_UNKNOW;
			if (type->valuestring){
				if(strcmp(type->valuestring, "INVALID DOMAIN") == 0){
					recog->error_type= AI_ERROR_INVALID_DOMAIN;
				}
			}
		}
		goto exit_error;
	}

	//	------------------------------------------------- domain
	domain = cJSON_GetObjectItem(param, "domain");
	if(domain){
		if (domain->valuestring){
			if(strcmp(domain->valuestring, "music") == 0){
				recog->domain = RECOG_DOMAIN_MUSIC;
				recog->env = "dlg_domain=\"音乐\";";
				if ((recog->search_title==NULL)
				  &&(recog->search_artist)){
					goto exit_slot;
				}
			}	//*/
			else if(strcmp(domain->valuestring, "chat") == 0){
				recog->domain = RECOG_DOMAIN_CHAT;
				recog->env = "dlg_domain=\"聊天\";";
			}
			else if(strcmp(domain->valuestring, "calendar") == 0){
				recog->domain = RECOG_DOMAIN_CALENDAR;
				recog->env = "dlg_domain=\"日历\";";
			}
			else if(strcmp(domain->valuestring, "weather") == 0){
				recog->domain = RECOG_DOMAIN_WEATHER;
				recog->env = "dlg_domain=\"天气\";";
			}
			else if(strcmp(domain->valuestring, "netfm") == 0){
				recog->domain = RECOG_DOMAIN_NETFM;
				recog->env = "dlg_domain=\"电台\";";
			}
			else if(strcmp(domain->valuestring, "command") == 0){
				recog->domain = RECOG_DOMAIN_COMMAND;
				recog->env = "dlg_domain=\"中控\";";
			}
			else if(strcmp(domain->valuestring, "reminder") == 0){
				recog->domain = RECOG_DOMAIN_REMINDER;
				recog->env = "dlg_domain=\"提醒\";";
			}
			else if(strcmp(domain->valuestring, "stock") == 0){
				recog->domain = RECOG_DOMAIN_STOCK;
				recog->env = "dlg_domain=\"股票\";";
			}
			else if(strcmp(domain->valuestring, "poetry") == 0){
				recog->domain = RECOG_DOMAIN_POETRY;
				recog->env = "dlg_domain=\"古诗\";";
			}

		}
	}

	if(recog->domain == RECOG_DOMAIN_COMMAND){

		goto exit_slot;
	}

	//-------------------------------------------------------         		state
	state = cJSON_GetObjectItem(param, "state");
	if(state){
	//	DEBUG("state : %s\n", state->valuestring);
		if (state->valuestring){
			if(strcmp(state->valuestring, "do") == 0){
				recog->state = SDS_STATE_DO;
			}
			else if(strcmp(state->valuestring, "query") == 0){
				recog->state = SDS_STATE_QUERY;
			}
			else if(strcmp(state->valuestring, "interact") == 0){
				recog->state = SDS_STATE_INTERACT;
			}
			else if(strcmp(state->valuestring, "offer") == 0){
				recog->state = SDS_STATE_OFFER;
			}
			else if(strcmp(state->valuestring, "offernone") == 0){
				recog->state = SDS_STATE_OFFERNONE;
			}
			else if(strcmp(state->valuestring, "exit") == 0){
				recog->state = SDS_STATE_EXIT;
			}
			else{
				recog->state = SDS_STATE_NULL;
			}
		}
	}	//*/
	//-------------------------------------------------------  				output
	output = cJSON_GetObjectItem(param, "output");
	if(output){
		if (output->valuestring){
			recog->output = output->valuestring;
		}
	}	//*/
	//-------------------------------------------------------  				contextId
	contextId = cJSON_GetObjectItem(param, "contextId");
	if(contextId){
		if (contextId->valuestring){
			recog->contextId = contextId->valuestring;
		}
	}	//*/
	//-------------------------------------------------------  				nlu
	nlu = cJSON_GetObjectItem(param, "nlu");
	if(nlu){
	    date = cJSON_GetObjectItem(nlu, "date");
	    time = cJSON_GetObjectItem(nlu, "time");
        if(date&&time){
		    if (date->valuestring&&time->valuestring){
			    recog->date = date->valuestring;
			    recog->time = time->valuestring;
	            event = cJSON_GetObjectItem(nlu, "event");
                if(event){
		            if (event->valuestring){
			            recog->event = event->valuestring;
                    }
                }
            }
        }
	}	//*/

	//-------------------------------------------------------  				data
	data = cJSON_GetObjectItem(param, "data");
	if(data){
		dbdata = cJSON_GetObjectItem(data, "dbdata");
		if(dbdata){
		//	DEBUG("PASS\n");
			if(recog->domain == RECOG_DOMAIN_WEATHER){
		//		DEBUG("PASS\n");
				int number = cJSON_GetArraySize(dbdata);
				DEBUG("number = %d\n",number);
				if (number ==1){
					dbdata_i = cJSON_GetArrayItem(dbdata, 0);
					area = cJSON_GetObjectItem(dbdata_i, "area");
					if(area){
						if (area->valuestring){
							recog->weather.area = area->valuestring;
						}
					}
					today = cJSON_GetObjectItem(dbdata_i, "today");
					if (today)
					{
						weather = cJSON_GetObjectItem(today, "weather");
						if(weather){
							if (weather->valuestring){
								recog->weather.weather = weather->valuestring;
							}
						}
						wind = cJSON_GetObjectItem(today, "wind");
						if(wind){
							if (wind->valuestring){
								recog->weather.wind = wind->valuestring;
							}
						}
						temperature = cJSON_GetObjectItem(today, "temperature");
						if(temperature){
							if (temperature->valuestring){
								recog->weather.temperature = temperature->valuestring;
							}
						}
					}
				}
			}
			else if((recog->domain == RECOG_DOMAIN_MUSIC)
			||(recog->domain == RECOG_DOMAIN_NETFM)
			||(recog->domain == RECOG_DOMAIN_CHAT)){
				recog->music_num = cJSON_GetArraySize(dbdata);
				int total_song=recog->music_num;
				int song_num = 1;
			//	get_nonce(song_num,total_song);
				DEBUG("total_song = %d,  play_num = %d\n",total_song,song_num);
				if ((song_num>0)&&(song_num<=total_song)){
					dbdata_i = cJSON_GetArrayItem(dbdata, song_num-1);

					if (dbdata_i){
						switch(recog->domain){
						//--------------------------------------------- get music url
						case RECOG_DOMAIN_MUSIC:
							url = cJSON_GetObjectItem(dbdata_i, "url");
							if(url){
								if (url->valuestring){
									recog->music.url = url->valuestring;
								}
								title = cJSON_GetObjectItem(dbdata_i, "title");
								if (title){
									if (title->valuestring){
										recog->music.title = title->valuestring;
									}
								}
								artist = cJSON_GetObjectItem(dbdata_i, "artist");
								if(artist){
									if (artist->valuestring){
										recog->music.artist = artist->valuestring;
									}
								}
							}
							break;
						//--------------------------------------------- get netfm url
						case RECOG_DOMAIN_NETFM:
							url_64 = cJSON_GetObjectItem(dbdata_i, "playUrl64");
							if(url_64){
								if (url_64->valuestring){
									recog->music.url = url_64->valuestring;
									title = cJSON_GetObjectItem(dbdata_i, "track");
									if (title){
										if (title->valuestring){
											recog->music.title = title->valuestring;
										}
									}
									break;
								}
							}
							url_32 = cJSON_GetObjectItem(dbdata_i, "playUrl32");
							if(url_32){
								if(url_32->valuestring){
									recog->music.url = url_32->valuestring;
									title = cJSON_GetObjectItem(dbdata_i, "track");
									if (title){
										if(title->valuestring){
											recog->music.title = title->valuestring;
										}
									}
								}
							}
							break;
						//--------------------------------------------- get char url
						case RECOG_DOMAIN_CHAT:
							url = cJSON_GetObjectItem(dbdata_i, "url");
							if(url){
								if (url->valuestring){
									recog->music.url = url->valuestring;
								}
							}
							break;
						}
					}
					else{
						PERROR("dbdata_i error!\n");
					}
				}
				else if(recog->domain != RECOG_DOMAIN_REMINDER){
					//DEBUG("song_num = %d ,error!\n",song_num);
				}
			}

		}
	}
	goto exit_slot;

exit_error:
	ai_slot_recog_free(recog);
exit_slot:
	DEBUG("input : %s\n", recog->input);
	DEBUG("output : %s\n", recog->output);
	DEBUG("contextId : %s\n", recog->contextId);
	DEBUG("domain : %s\n", aiSlotDomain[recog->domain]);
	DEBUG("state : %s\n", aiSlotState[recog->state]);
	DEBUG("operation : %s\n", recog->operation);
	DEBUG("device : %s\n", recog->device);
	DEBUG("location : %s\n", recog->location);
	DEBUG("object : %s\n", recog->object);
	DEBUG("date : %s\n", recog->date);
	DEBUG("time : %s\n", recog->time);
	DEBUG("event : %s\n", recog->event);

	DEBUG("search_artist : %s\n", recog->search_artist);//*/
	DEBUG("search_title : %s\n", recog->search_title);//*/
	DEBUG("url_number : %d\n", recog->music_num);
	DEBUG("url : %s\n", recog->music.url);
	DEBUG("artist : %s\n", recog->music.artist);
	DEBUG("title : %s\n", recog->music.title);//*/
	DEBUG("area : %s\n", recog->weather.area);//*/
	DEBUG("weather : %s\n", recog->weather.weather);//*/
	DEBUG("temperature : %s\n", recog->weather.temperature);//*/
	DEBUG("wind : %s\n", recog->weather.wind);//*/
	DEBUG("movie name : %s\n", recog->movie.name);//*/
	DEBUG("movie sequence : %s\n", recog->movie.sequence);//*/
	DEBUG("movie director : %s\n", recog->movie.director);//*/
	DEBUG("movie player : %s\n", recog->movie.player);//*/
	DEBUG("movie type : %s\n", recog->movie.type);//*/
	DEBUG("movie area : %s\n", recog->movie.area);//*/

/*	if (param != NULL){
		cJSON_Delete(param);
		param = NULL;
	}	//*/
	if (recog->domain == RECOG_DOMAIN_NULL)
		ret = -1;

	if (ret)
		recog->status = AIENGINE_STATUS_ERROR;

	return ret;
}

#endif

