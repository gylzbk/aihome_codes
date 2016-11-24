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
#include "aiengine_app.h"
#include "ai_server.h"
#include "ai_error.h"

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



int ai_server_fun(vr_info *recog)
{
	int error = 0;
	char str[256]={0};
	int command = SDS_COMMAND_NULL;
	recog->next_status     = AIENGINE_STATUS_AEC;

	if (recog == NULL){
		recog->error_type = AI_ERROR_SYSTEM;
		recog->next_status    = AIENGINE_STATUS_ERROR;
		error = -1;
		goto exit_error;
	}	//*/
//-------------------------------------------------- sem error
    /***Add sem timeout prompt to restart to wakeup after aoubt 8s by Ray Zhang***/
	if((NULL == recog->input)
	||(strlen(recog->input) == 0)){
		recog->next_status     = AIENGINE_STATUS_ERROR;
		recog->error_type = AI_ERROR_NO_VOICE;
		error = -1;
		goto exit_error;
	}

    /***Add sem timeout prompt to restart to wakeup after aoubt 8s by Ray Zhang***/
	if(SDS_STATE_EXIT==recog->state){
		recog->next_status     = AIENGINE_STATUS_AEC;
		goto exit_error;
	}
	if (recog->operation){
		if((strcmp(recog->operation, "退出") == 0)
		||(strcmp(recog->operation, "结束") == 0)){
			recog->domain = RECOG_DOMAIN_COMMAND;
			recog->next_status  = AIENGINE_STATUS_AEC;
			command = SDS_COMMAND_EXIT;
			goto exit_command;
		}
	}

	#if SUPPORT_ELIFE
		elife_t *elife = ai_elife_server(recog);
		if (elife->resp.msg){
			ai_tts(elife->resp.msg,false);
		}
		if (elife->resp.is_cy){					//	elife do
			if (elife->resp.is_mult){			//	elife mult
				recog->next_status  = AIENGINE_STATUS_SEM;
			}
			else{
				recog->next_status  = AIENGINE_STATUS_AEC;
			}
			goto exit_error;
		}
	#endif


	switch(recog->domain){
	//	----------------------------------------  music
		case RECOG_DOMAIN_MUSIC:
	//	----------------------------------------  netfm
		case RECOG_DOMAIN_NETFM:
			if (recog->domain== RECOG_DOMAIN_MUSIC){
			//-------------------- play music-- only artist
				if((recog->search_artist)
					&&(recog->search_title == NULL)){
					DEBUG("-------------Only search artist!...\n");
					ai_song_list_renew_artist(recog->search_artist);
					ai_aitalk_send(aitalk_send_next_music(false));
					recog->next_status     = AIENGINE_STATUS_AEC;
					goto	exit_error;
					break;
				}
			}
			//-------------------- play music-- music name
			if ((recog->music.url)&&(recog->music.title)){
				char str[512]={0};
				if (recog->music.artist){
					strcpy(str,recog->music.artist);
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
				}
			#if AI_CONTROL_MOZART
				music_info music;
				music.url = recog->music.url;
				music.artist = recog->music.artist;
				music.title = recog->music.title;
				//ai_music_list_add_music(music);
				recog->is_control_play_music = true;
				ai_aitalk_send(aitalk_send_play_url(&music));
				ai_song_list_renew_auto();
			#endif
				recog->next_status     = AIENGINE_STATUS_AEC;
				break;
			}
			switch(recog->state)
			{
				case SDS_STATE_DO:
					recog->next_status     = AIENGINE_STATUS_AEC;
				break;

				case SDS_STATE_OFFERNONE:
					recog->next_status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;

				case SDS_STATE_OFFER:
					recog->next_status     = AIENGINE_STATUS_AEC;
				break;

				case SDS_STATE_INTERACT:
					recog->next_status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
				default:
					recog->next_status     = AIENGINE_STATUS_AEC;
					break;
			}
			if (recog->output){
				if (recog->next_status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,false);
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
					recog->next_status     = AIENGINE_STATUS_AEC;
				break;
				case SDS_STATE_INTERACT:
					recog->next_status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
			}
			if (recog->output){
				if (recog->next_status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,false);
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
						recog->next_status     = AIENGINE_STATUS_AEC;
					}
					else{
						recog->next_status     = AIENGINE_STATUS_SEM;
					}
				break;
				case SDS_STATE_INTERACT:
					recog->next_status     = AIENGINE_STATUS_SEM;
					recog->sds_flag = 1;
				break;
				default:
					recog->next_status     = AIENGINE_STATUS_SEM;
					break;
			}

			if (recog->output){
				if (recog->next_status == AIENGINE_STATUS_SEM){
					ai_tts(recog->output,false);
				}
				else{
					ai_tts(recog->output,false);
				}
			}
			break;
	//	----------------------------------------  chat
		case RECOG_DOMAIN_CHAT:
		//	ai_flag.is_play_music = false;
			if (recog->music.url){
				DEBUG("url=%s\n",recog->music.url);
			#if AI_CONTROL_MOZART
				music_info music;
				music.url = recog->music.url;
				music.artist = recog->music.artist;
				music.title = recog->music.title;
				recog->is_control_play_music = true;
				ai_aitalk_send(aitalk_send_play_url(&music));
			#endif
				recog->next_status     = AIENGINE_STATUS_AEC;
			}	//*/
			else{
				if (recog->output){
					ai_tts(recog->output,false);
				}
				recog->next_status     = AIENGINE_STATUS_SEM;
			}
			break;
	//	----------------------------------------weather
		case RECOG_DOMAIN_WEATHER:
			switch(recog->state){
				case SDS_STATE_DO:
				case SDS_STATE_OFFERNONE:
					if (recog->output){
						ai_tts(recog->output,true);
						recog->is_control_play_music = true;
					}
					recog->next_status     = AIENGINE_STATUS_AEC;
					break;
                case SDS_STATE_INTERACT:
					if (recog->output){
						ai_tts(recog->output,false);
					}
					recog->sds_flag = 1;
					recog->next_status     = AIENGINE_STATUS_SEM;
					break;
				default:
					break;
			}
			break;

	//	----------------------------------------    movie
		case RECOG_DOMAIN_MOVIE:
			recog->next_status     = AIENGINE_STATUS_AEC;
		/*	#if SUPPORT_ELIFE
				ai_elife_movie_free();
				if (recog->operation){
					elife.cmd = strdup(recog->operation);
				}
				if (recog->movie.name){
					elife.movie.name = strdup(recog->movie.name);
				}
				elife.movie.sequence = atoi(recog->movie.sequence);

				if (recog->movie.director){
					elife.movie.director = strdup(recog->movie.director);
				}
				if (recog->movie.player){
					elife.movie.player = strdup(recog->movie.player);
				}
				if (recog->movie.type){
					elife.movie.type = strdup(recog->movie.type);
				}
				if (recog->movie.area){
					elife.movie.area= strdup(recog->movie.area);
				}
				ai_elife_movie();
			#endif
			//*/
		break;
	//	----------------------------------------    command
		case RECOG_DOMAIN_COMMAND:
			recog->next_status     = AIENGINE_STATUS_AEC;
		break;
		default:
			PERROR("Unknown domain!\n");
			recog->error_type = AI_ERROR_INVALID_DOMAIN;
			recog->next_status    =AIENGINE_STATUS_ERROR;
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
				char *tone_key;
				if(strcmp(recog->volume, "+") == 0){
					tone_key = "volume_up";
				}
				else if(strcmp(recog->volume, "-") == 0){
					tone_key = "volume_down";
				}
				else if(strcmp(recog->volume, "max") == 0){
					tone_key = "volume_max";
					free(recog->volume);
					recog->volume = NULL;
					recog->volume = strdup("100");
				}	//*/
				else if(strcmp(recog->volume, "min") == 0){
					tone_key = "volume_min";
					free(recog->volume);
					recog->volume = NULL;
					recog->volume = strdup("0");
				}	//*/
				else {
					strcpy(str,"为您设置音量为");
					strcat (str,recog->volume);
					ai_tts(str,false);
					tone_key = "";
				}
				#if AI_CONTROL_MOZART
				ai_aitalk_send(aitalk_send_set_volume(recog->volume,tone_key));
				#endif
			}
			break;
#if AI_CONTROL_MOZART
		case SDS_COMMAND_MUSIC_PAUSE:
		//	mozart_prompt_tone_key_sync("pause",false);
		//	ai_flag.is_play_music = false;
		//	if (mozart_module_is_playing()==1){
			recog->is_control_play_music = true;
			ai_aitalk_send(aitalk_send_pause(true));
		//	}
			break;
		case SDS_COMMAND_MUSIC_RESUME:
		//	mozart_prompt_tone_key_sync("resume",false);
		//	ai_flag.is_play_music = true;

			recog->is_control_play_music = true;
			ai_aitalk_send(aitalk_send_resume(true));
			break;
		case SDS_COMMAND_MUSIC_STOP:
		//	ai_flag.is_play_music = false;
		//	mozart_prompt_tone_key_sync("stop",false);
			recog->is_control_play_music = true;
			ai_aitalk_send(aitalk_send_stop_music(true));
			break;
		case SDS_COMMAND_MUSIC_PLAY:
			if ((recog->music.url == NULL)&&(recog->output== NULL)){
		//		ai_flag.is_play_music = true;
				recog->is_control_play_music = true;
				ai_aitalk_send(aitalk_send_resume(true));
			}
			break;
		case SDS_COMMAND_MUSIC_PREVIOUS:
			recog->is_control_play_music = true;
			ai_aitalk_send(aitalk_send_previous_music(true));
		//	mozart_prompt_tone_key_sync("previous",false);
		//	ai_play_music_order(-1);
			break;
		case SDS_COMMAND_MUSIC_NEXT:
			recog->is_control_play_music = true;
			ai_aitalk_send(aitalk_send_next_music(true));
			break;
		case SDS_COMMAND_EXIT:
		//	DEBUG("PASS\n");
			recog->is_control_play_music = false;
			ai_aitalk_send(aitalk_send_exit(NULL));
		//	mozart_prompt_tone_key_sync("exit",false);
			break;
		#if SUPPORT_ELIFE
		case SDS_COMMAND_ELIFE:
		/*	DEBUG("Start elfe command!...\n");
			ai_elife_command_free();
			if (recog->operation){
				elife.cmd = strdup(recog->operation);
			}
			if (recog->device){
				elife.name = strdup(recog->device);
			}
			if (recog->location){
				elife.position = strdup(recog->location);
			}
			ai_elife_command();
			break;	//*/
		#endif
#endif
		default:
			break;
		}
	}

exit_error:
//-------------------------------------------------------	释放分配的内存
	if (error<0){
		recog->next_status    = AIENGINE_STATUS_ERROR;
	}
	if((recog->key_record_stop == true)
	|| (aiengine_ini.sds.is_multi == false)){	//	add sds multi check at  161106
		recog->status = AIENGINE_STATUS_AEC;
 	} else {
		recog->status = recog->next_status    ;
	}
	recog->key_record_stop = false;
   	return error;
}

int ai_server_init(void)
{
	#if AI_LOG_ENABLE
	ai_log_init();
	#endif
	//ai_music_list_init();
//	ai_song_recommend_init();
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
	//ai_music_list_free();
//	ai_song_recommend_free_all();
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

