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
#include <stdbool.h>
#include "ai_music_list.h"
#include "aiengine_app.h"
#include "ai_server.h"
#include "ai_error.h"

#include "ai_elife_doss.h"

#include "vr-speech_interface.h"

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}
#if 0
//--------------------------------------------------- prompt something
int ai_server_prompt(vr_info *recog,char *prompt){
	if (prompt == NULL){
		return -1;
	}

	free(recog->output);
	recog->output = strdup(prompt);
	if(recog->output)
		recog->status = AIENGINE_STATUS_TTS_ANSWER;
	else
		recog->status = AIENGINE_STATUS_AEC;


	return 0;
}
#endif
#if 0
static char *play_prompt = NULL;

int ai_domain_update(int domain){
//	char *prompt = NULL;
	ai_domain = domain;
	switch(ai_domain){
		case RECOG_DOMAIN_MUSIC:
			play_prompt = "音 乐";
		break;
		case RECOG_DOMAIN_CHAT:
			play_prompt = "闲 聊";
			break;
		case RECOG_DOMAIN_WEATHER:
			play_prompt = "天 气";
			break;
		case RECOG_DOMAIN_CALENDAR:
			play_prompt = "日 历";
			break;
		case RECOG_DOMAIN_NETFM:
			play_prompt = "电 台";
			break;
		case RECOG_DOMAIN_STOCK:
			play_prompt = "股 票";
			break;
		case RECOG_DOMAIN_POETRY:
			play_prompt = "诗 歌";
			break;
		default:
			play_prompt = NULL;
			break;
	}

	/*if (prompt){
		free(play_prompt);
		play_prompt = NULL;
		play_prompt = strdup(prompt);
		pr_debug("\ndomain=%d,prompt=%s,play_prompt=%s\n",domain,prompt,play_prompt);
	}//*/
	pr_debug("\ndomain=%d,play_prompt=%s\n",domain,play_prompt);
	return 0;
}
#endif

int ai_server_fun(vr_info *recog)
{
	int error = 0;
	char str[256]={0};
	int command = SDS_COMMAND_NULL;
	if (recog == NULL){
		recog->error_type = AI_ERROR_SYSTEM;
		recog->status    = AIENGINE_STATUS_ERROR;
		error = -1;
		goto exit_error;
	}	//*/
//-------------------------------------------------- sem error
#if 0
	//-------------------------------------------- long work test
	recog->status     = AIENGINE_STATUS_AEC;
	DEBUG("DEBUG: long work test \n");
	goto exit_error;
#endif
    /***Add sem timeout prompt to restart to wakeup after aoubt 8s by Ray Zhang***/
	if(NULL == recog->input){
		recog->status     = AIENGINE_STATUS_ERROR;
		recog->error_type = AI_ERROR_NO_VOICE;
		error = -1;
		goto exit_error;
	}
    /***Add sem timeout prompt to restart to wakeup after aoubt 8s by Ray Zhang***/
	if(SDS_STATE_EXIT==recog->state){
		recog->status     = AIENGINE_STATUS_AEC;
		goto exit_error;
	}
	if (recog->operation){
		if((strcmp(recog->operation, "退出") == 0)
		||(strcmp(recog->operation, "结束") == 0)){
			recog->domain = RECOG_DOMAIN_COMMAND;
			recog->status  = AIENGINE_STATUS_AEC;
			command = SDS_COMMAND_EXIT;
			goto exit_command;
		}
	}

	switch(recog->domain){
	//	----------------------------------------  music
		case RECOG_DOMAIN_MUSIC:
	//	----------------------------------------  netfm
		case RECOG_DOMAIN_NETFM:
			if (recog->domain== RECOG_DOMAIN_MUSIC){
//------------------------------ only artist
				if((recog->search_artist)
					&&(recog->search_title == NULL)){
					DEBUG("-------------Only search artist!...\n");
				//	ai_tts(recog->search_artist,false);
					ai_music_list_free();
					ai_song_recommend_artist(recog->search_artist);
					#if AI_CONTROL_MOZART
					aitalk_pipe_put(aitalk_play_music_json(NULL));
					#endif
					recog->status     = AIENGINE_STATUS_AEC;
					goto	exit_error;
						break;
				}
			}
			if ((recog->music.url)&&(recog->music.title)){
				char str[512]={0};
			//	strcpy(str,"准备播放");
				if (recog->music.artist){
					strcpy(str,recog->music.artist);
				//	strcat (str,recog->music.artist);
					strcat(str,",");
					strcat (str,recog->music.title);
				}
				else{
					strcpy(str,recog->music.title);
				}
				ai_tts(str,false);
			}
			if (recog->music.url){
				if (recog->domain == RECOG_DOMAIN_MUSIC){
					ai_music_list_add_music(&recog->music);
				}
			#if AI_CONTROL_MOZART
				music_info music;
				music.url = recog->music.url;
				music.artist = recog->music.artist;
				music.title = recog->music.title;
				aitalk_pipe_put(aitalk_play_url_json(&music));
			#endif
				recog->status     = AIENGINE_STATUS_AEC;
				break;
			}
			switch(recog->state)
			{
				case SDS_STATE_DO:
					recog->status     = AIENGINE_STATUS_AEC;
				break;

				case SDS_STATE_OFFERNONE:
					recog->status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;

				case SDS_STATE_OFFER:
					recog->status     = AIENGINE_STATUS_AEC;
				break;

				case SDS_STATE_INTERACT:
					recog->status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
				default:
					recog->status     = AIENGINE_STATUS_AEC;
					break;
			}
			if (recog->output){
				if (recog->status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,true);
				}
			}
		break;
	//	----------------------------------------  calendar
	//	----------------------------------------  stock
	//	----------------------------------------  reminder
		case RECOG_DOMAIN_CALENDAR:
		case RECOG_DOMAIN_STOCK:
		case RECOG_DOMAIN_REMINDER:
			switch(recog->state)
			{
				case SDS_STATE_DO:
					recog->status     = AIENGINE_STATUS_AEC;
				break;
				case SDS_STATE_INTERACT:
					recog->status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
			}
			if (recog->output){
				if (recog->status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,true);
				}
			}
		break;

	//	----------------------------------------  poetry
		case RECOG_DOMAIN_POETRY:
			switch(recog->state)
			{
				case SDS_STATE_DO:
					if (recog->music.url){
						DEBUG("url = %s\n",recog->music.url);
						recog->status     = AIENGINE_STATUS_AEC;
					}
					else{
						recog->status     = AIENGINE_STATUS_SEM;
					}
				break;
				case SDS_STATE_INTERACT:
					recog->status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
				default:
					recog->status     = AIENGINE_STATUS_SEM;
					break;
			}

			if (recog->output){
				if (recog->status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,true);
				}
			}
			break;
	//	----------------------------------------  chat
		case RECOG_DOMAIN_CHAT:
		//	aitalk_play_music = false;
			if (recog->music.url){
				DEBUG("url=%s\n",recog->music.url);
			#if AI_CONTROL_MOZART
				music_info music;
				music.url = recog->music.url;
				music.artist = recog->music.artist;
				music.title = recog->music.title;
				aitalk_pipe_put(aitalk_play_url_json(&music));
			#endif
				recog->status     = AIENGINE_STATUS_AEC;
			}	//*/
			else{
				recog->status     = AIENGINE_STATUS_SEM;
				if (recog->output){
					ai_tts(recog->output,false);
				}
			}
			break;
	//	----------------------------------------weather
		case RECOG_DOMAIN_WEATHER:
			switch(recog->state){
				case SDS_STATE_DO:
				case SDS_STATE_OFFERNONE:
					recog->status     = AIENGINE_STATUS_AEC;
					if (recog->output){
						ai_tts(recog->output,true);
					}
					break;
                case SDS_STATE_INTERACT:
					recog->sds_flag = 1;
					recog->status     = AIENGINE_STATUS_SEM;
					if (recog->output){
						ai_tts(recog->output,false);
					}
					break;
				default:
					break;
			}
			break;

	//	----------------------------------------    movie
		case RECOG_DOMAIN_MOVIE:
			recog->status     = AIENGINE_STATUS_AEC;
		#if SUPPORT_ELIFE
				ai_elife_movie_free();
				if (recog->operation){
					ai_elife.cmd = strdup(recog->operation);
				}
				if (recog->movie.name){
					ai_elife.movie.name = strdup(recog->movie.name);
				}
				ai_elife.movie.sequence = recog->movie.sequence;

				if (recog->movie.director){
					ai_elife.movie.director = strdup(recog->movie.director);
				}
				if (recog->movie.player){
					ai_elife.movie.player = strdup(recog->movie.player);
				}
				if (recog->movie.type){
					ai_elife.movie.type = strdup(recog->movie.type);
				}
				if (recog->movie.area){
					ai_elife.movie.area= strdup(recog->movie.area);
				}
				ai_elife_movie();
		#endif
		break;
	//	----------------------------------------    command
		case RECOG_DOMAIN_COMMAND:
			recog->status     = AIENGINE_STATUS_AEC;
		break;
		default:
			PERROR("Unknown domain!\n");
			recog->error_type = AI_ERROR_INVALID_DOMAIN;
			recog->status    =AIENGINE_STATUS_ERROR;
			break;
	}	//*/

	if((RECOG_DOMAIN_COMMAND == recog->domain)
	||(RECOG_DOMAIN_MUSIC== recog->domain)){
		if(recog->volume){
			command = SDS_COMMAND_VOLUME;
		}
		else{
			//---------------------------- operation -> command_type
			if(recog->operation){
			 	if(strcmp(recog->operation, "暂停") == 0){
					command = SDS_COMMAND_MUSIC_PAUSE;
				}
				else if(strcmp(recog->operation, "播放") == 0){
					command = SDS_COMMAND_MUSIC_PLAY;
				}
				else if(strcmp(recog->operation, "继续") == 0){
					command = SDS_COMMAND_MUSIC_RESUME;
				}
				else if(strcmp(recog->operation, "停止") == 0){
					command = SDS_COMMAND_MUSIC_STOP;
				}
				else if(strcmp(recog->operation, "上一首") == 0){
					command = SDS_COMMAND_MUSIC_PREVIOUS;
				}
				else if(strcmp(recog->operation, "下一首") == 0){
					command = SDS_COMMAND_MUSIC_NEXT;
				}
				else if(strcmp(recog->operation, "退出") == 0){
					command = SDS_COMMAND_EXIT;
				}
				else if(strcmp(recog->operation, "结束") == 0){
					command = SDS_COMMAND_EXIT;
				}
				else{
				#if SUPPORT_ELIFE
					command = SDS_COMMAND_ELIFE;
				#endif
				}
			}
		}

exit_command:
		switch(command){
		case SDS_COMMAND_VOLUME:
			if (recog->volume){
				if(strcmp(recog->volume, "+") == 0){
					#if AI_CONTROL_MOZART
					mozart_prompt_tone_key_sync("volume_up", false);
					#endif
				}
				else if(strcmp(recog->volume, "-") == 0){
					#if AI_CONTROL_MOZART
					mozart_prompt_tone_key_sync("volume_down", false);
					#endif
				}
				else if(strcmp(recog->volume, "max") == 0){
					#if AI_CONTROL_MOZART
					mozart_prompt_tone_key_sync("volume_max", false);
					#endif
					free(recog->volume);
					recog->volume = strdup("100");
				}	//*/
				else if(strcmp(recog->volume, "min") == 0){
					#if AI_CONTROL_MOZART
					mozart_prompt_tone_key_sync("volume_min", false);
					#endif
					free(recog->volume);
					recog->volume = strdup("0");
				}	//*/
				else {
					strcpy(str,"为您设置音量为");
					strcat (str,recog->volume);
					ai_tts(str,false);
				}
				#if AI_CONTROL_MOZART
				aitalk_pipe_put(aitalk_set_volume_json(recog->volume));
				#endif
			}
			break;
#if AI_CONTROL_MOZART
DEBUG("PASS\n");
		case SDS_COMMAND_MUSIC_PAUSE:
			mozart_prompt_tone_key_sync("pause", false);
		//	aitalk_play_music = false;
			if (mozart_module_is_playing()==1){
				aitalk_pipe_put(aitalk_pause_json(NULL));
			}
			break;
		case SDS_COMMAND_MUSIC_RESUME:
			mozart_prompt_tone_key_sync("resume", false);
		//	aitalk_play_music = true;
			aitalk_pipe_put(aitalk_resume_json(NULL));
			break;
		case SDS_COMMAND_MUSIC_STOP:
		//	aitalk_play_music = false;
			mozart_prompt_tone_key_sync("stop", false);
			aitalk_pipe_put(aitalk_stop_music_json(NULL));
			break;
		case SDS_COMMAND_MUSIC_PLAY:
			if ((recog->music.url == NULL)&&(recog->output== NULL)){
		//		aitalk_play_music = true;
				mozart_prompt_tone_key_sync("resume", false);
				aitalk_pipe_put(aitalk_play_music_json(NULL));
			}
			break;
		case SDS_COMMAND_MUSIC_PREVIOUS:
			mozart_prompt_tone_key_sync("previous", false);
			aitalk_pipe_put(aitalk_previous_music_json(NULL));
			break;
		case SDS_COMMAND_MUSIC_NEXT:
			mozart_prompt_tone_key_sync("next", false);
			aitalk_pipe_put(aitalk_next_music_json(NULL));
			break;
		case SDS_COMMAND_EXIT:
			DEBUG("PASS\n");
			mozart_prompt_tone_key_sync("exit", false);
			break;
		#if SUPPORT_ELIFE
		case SDS_COMMAND_ELIFE:
			ai_elife_command_free();
			if (recog->operation){
				ai_elife.cmd = strdup(recog->operation);
			}
			if (recog->device){
				ai_elife.name = strdup(recog->device);
			}
			if (recog->location){
				ai_elife.position = strdup(recog->location);
			}
			ai_elife_command();
			break;
		#endif
#endif
		default:
			break;
		}
	}

exit_error:
//-------------------------------------------------------	释放分配的内存
	if (error<0){
		recog->status    = AIENGINE_STATUS_ERROR;
	}
//	ai_slot_recog_free(recog);
   	return error;
}

int ai_server_init(void){
//	ai_curlInit();
	ai_music_list_init();
	ai_song_recommend_init();
//	ai_song_recommend_auto();
	#if  SUPPORT_ELIFE
	ai_elife_init();
	#endif
//	ai_output_init();
//	ai_output_url("深圳天气怎样？");
//	ai_song_recommend_push();

	//int i ;
	//for(i=0;i<25;i++){
	//	ai_song_recommend_push();
	//}
	return 0;
}

int ai_server_exit(void){
	ai_music_list_free();
	ai_song_recommend_free_all();
	#if  SUPPORT_ELIFE
	ai_elife_free();
	#endif
	return 0;
}

int ai_server_restart(void){
	DEBUG("\n\n============================= ai server restart now...\n");
	ai_server_exit();
	ai_server_init();
	return 0;
}

