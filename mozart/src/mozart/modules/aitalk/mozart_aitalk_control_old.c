#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <json-c/json.h>

#include "utils_interface.h"
#include "player_interface.h"
#include "volume_interface.h"
#include "linklist_interface.h"
#include "tips_interface.h"
#include "event_interface.h"
#include "localplayer_interface.h"

#include "mozart_module.h"
#include "mozart_misc.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_net.h"
#include "mozart_aitalk.h"
#include "mozart_player.h"
#include "mozart_aitalk_control.h"
//#include "mozart_led.h"
//#include "mozart_key.h"

#include "vr-speech_interface.h"
//#include "mozart_license.h"
#include "aiengine_app.h"
#include <semaphore.h>

#ifndef MOZART_RELEASE
#define MOZART_AITALK_CONTROL_DEBUG
#endif

#ifdef MOZART_AITALK_CONTROL_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[AITALK_CONTROL] %s: "fmt, __func__, ##args)
#else  /* MOZART_AITALK_CONTROL_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_AITALK_CONTROL_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[AITALK_CONTROL] [Error] %s: "fmt, __func__, ##args)
#define pr_info(fmt, args...)			\
	printf("[AITALK_CONTROL] [Info] %s: "fmt, __func__, ##args)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
struct aitalk_method {
	const char *name;
	int (*handler)(json_object *cmd);
	bool (*is_valid)(json_object *cmd);
};


enum aitalk_wait_stop_enum {
	aitalk_wait_stop_invalid,
	aitalk_wait_stop_waitstop,
	aitalk_wait_stop_stopped,
} aitalk_wait_stop_state;
static pthread_mutex_t aitalk_wait_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t aitalk_wait_stop_cond = PTHREAD_COND_INITIALIZER;
//*/

static char *play_prompt = NULL;
static char *current_url = NULL;
//char *play_url;
//static int set_volume;
bool aitalk_play_music = false;
static bool aitalk_is_playing;
static bool aitalk_initialized = false;
static player_handler_t *aitalk_player_handler;
static int ai_domain;
//static int ai_domain_porte;

aitalk_headle_e aitalk_do_handler;

int aitalk_stop_player(void)
{
	long usec;
	struct timeval now;
	struct timespec timeout;

	if (!aitalk_is_playing)
		return 0;

	pthread_mutex_lock(&aitalk_wait_stop_mutex);

	aitalk_wait_stop_state = aitalk_wait_stop_waitstop;

	if (mozart_player_stop(aitalk_player_handler))
		printf("[Warning] %s: mozart_player_stop fail\n", __func__);

	gettimeofday(&now, NULL);
	usec = now.tv_usec + 500 * 1000;
	timeout.tv_sec = now.tv_sec + usec / 1000000;
	timeout.tv_nsec = (usec % 1000000) * 1000;

	pthread_cond_timedwait(&aitalk_wait_stop_cond, &aitalk_wait_stop_mutex, &timeout);
	if (aitalk_wait_stop_state != aitalk_wait_stop_stopped) {
		pr_err("wait stopped timeout\n");
		mozart_player_force_stop(aitalk_player_handler);
		aitalk_is_playing = false;
	}

	aitalk_wait_stop_state = aitalk_wait_stop_invalid;

	pthread_mutex_unlock(&aitalk_wait_stop_mutex);

	aitalk_play_music = false;
	return 0;
}

int __aitalk_switch_mode(bool attach)
{
	int ret = 0;

	if (attach) {
		if (!__mozart_module_is_attach()) {
			__mozart_module_set_attach();
		//	ret = send_button_event("switch_mode", NULL,"attach", "true");
		}
	} else {
		if (__mozart_module_is_attach()) {
			__mozart_module_set_unattach();
		//	ret = send_button_event("switch_mode", NULL,"attach", "false");
		}
	}

	return ret;
}



enum player_state {
	player_play_state = 0,
	player_pause_state,
	player_stop_state,
};
int aitalk_player_state = player_stop_state;

static int send_player_state_change(enum player_state state)
{
	aitalk_player_state = state;
	return 0;
}

static int send_play_done(const char *url, int error_no)
{
//	pr_debug("Add url into music list...(use aiengien server)\n");
/* for __stop_handler */
	if (aitalk_play_music == true){	//----------- conture playing
		aitalk_pipe_put(aitalk_send_next_music(NULL));
	}
	else{
		aitalk_is_playing = false;
	}

	return 0;
}

bool play_error_tone = false;
int aitalk_play_music_order(int order){
	music_info *music = NULL;
	music = ai_music_list_play_order(order);
//	pr_debug("url %s\n",url);
	if ((music)&&(music->url)){
		aitalk_pipe_put(aitalk_send_play_url(music));
		play_error_tone = false;
		return 0;
	}

	aitalk_play_music = false;
	if (play_error_tone == false){
		play_error_tone = true;
		mozart_prompt_tone_key_sync("error_net_fail", false);
	}

	return -1;
}

/*******************************************************************************
 * handler
 *******************************************************************************/
/*static bool vendor_is_valid(json_object *cmd)
{
	return aitalk_monitor_is_valid();
}	//*/


static int play_handler(json_object *cmd)
{
	int ret;
	const char *url;
	json_object *params, *url_j, *artist, *title;

	if (!json_object_object_get_ex(cmd, "params", &params)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "url", &url_j)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "title", &title)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "artist", &artist)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}

	url = json_object_get_string(url_j);
	if((url == NULL)
	||(url[0] == '/' && access(url, R_OK))){
		pr_err("[%d]error!\n",__LINE__);
		send_play_done(url, 0);
		return 0;
	}
#if SUPPORT_SMARTUI
	pr_debug("play_prompt = %s\n",play_prompt);
	char *title_s =(char *)json_object_get_string(title);
	char *artist_s =  (char *)json_object_get_string(artist);

	if((artist_s)||(artist_s)){
		mozart_smartui_atalk_play("AIspeech",title_s,artist_s,play_prompt);
	}
#endif
	ret = mozart_aitalk_do_play();
	if (ret == 0) {
		pr_debug("aitalk module isn't run\n");
	} else if (ret < 0) {
		pr_debug("aitalk module isn't start\n");
		send_player_state_change(player_pause_state);
		return -1;
	}

	free(current_url);
	current_url = strdup(url);
//	ai_tts_play_time();
//	return 0;		//	test long memory
	if (ret > 0) {
		if (mozart_player_playurl(aitalk_player_handler, (char *)url)){
			printf("[Warning] %s: mozart_player_playurl fail\n", __func__);
		}
	//	pr_debug("%s line = %d url: %s\n",__func__, __LINE__,url);
		send_player_state_change(player_play_state);
	} else {
		char *uuid = mozart_player_getuuid(aitalk_player_handler);
		mozart_aitalk_update_context(uuid, (char *)url);
		send_player_state_change(player_pause_state);
		free(uuid);
	}
	aitalk_is_playing = true;
	return 0;
}

static int pause_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	mozart_aitalk_do_pause();
	return 0;
}

static int resume_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	mozart_aitalk_do_resume();
	return 0;
}

static int stop_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	mozart_aitalk_do_stop();
	return 0;
}

static int pause_toggle_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	mozart_aitalk_do_resume_pause();

	return 0;
}

static int play_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	aitalk_play_music_order(0);
	return 0;
}


static int previous_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	aitalk_play_music_order(-1);
	return 0;
}

static int next_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	aitalk_play_music_order(1);
	return 0;
}


static int set_volume_handler(json_object *cmd)
{
	const char *volume;
	int vol = 0;
	printf("%s...!\n",__func__);
	json_object *params, *volume_j;//, *artist, *title, *vendor;

	if (!json_object_object_get_ex(cmd, "params", &params)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "volume", &volume_j)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}

	volume = json_object_get_string(volume_j);
	if (volume){
		vol = mozart_volume_get();
		pr_debug("volume = %s, last vol = %d!\n",volume,vol);
		if(strcmp(volume, "+") == 0){
			vol +=10;
		}else if(strcmp(volume, "-") == 0){
			vol -= 10;
		}else{
			vol = atoi(volume);
		}	//*/
		if (vol > 100)
			vol = 100;
		else if (vol < 0)
			vol = 0;

		pr_debug("new vol = %d!\n",vol);
		mozart_volume_set(vol, MUSIC_VOLUME);
	}
	else{
		return -1;
	}
	if(aitalk_play_music == true){
		mozart_aitalk_do_resume();
	}
	return 0;
}

static struct aitalk_method methods[] = {
	{
		.name = "play",
		.handler = play_handler,
	//	.is_valid = vendor_is_valid,
	},
	{
		.name = "stop",
		.handler = stop_handler,
	},

	{
		.name = "pause",
		.handler = pause_handler,
	},

	{
		.name = "resume",
		.handler = resume_handler,
	},

	{
		.name = "stop",
		.handler = stop_handler,
	},

	{
		.name = "pause_toggle",
		.handler = pause_toggle_handler,
	},

	{
		.name = "play_music",
		.handler = play_music_handler,
	},

	{
		.name = "previous_music",
		.handler = previous_music_handler,
	},

	{
		.name = "next_music",
		.handler = next_music_handler,
	},

	{
		.name = "set_volume",
		.handler = set_volume_handler,
	//	.is_valid = module_is_attach,
	},
};

/*******************************************************************************
 * Player
 *******************************************************************************/
static int aitalk_player_status_callback(player_snapshot_t *snapshot,
					struct player_handler *handler, void *param)
{
	if ((handler->uuid)&&(snapshot->uuid)){
		if (strcmp(handler->uuid, snapshot->uuid))
			return 0;
	}

	pr_debug("status = %d, %s\n", snapshot->status, player_status_str[snapshot->status]);

	if (snapshot->status == PLAYER_STOPPED) {
		pthread_mutex_lock(&aitalk_wait_stop_mutex);
		if (aitalk_wait_stop_state == aitalk_wait_stop_waitstop) {
			pr_debug("aitalk_wait_stop_state is WAIT_STOP\n");
			aitalk_wait_stop_state = aitalk_wait_stop_stopped;
		} else {
			send_play_done(current_url, 0);
		}
		pthread_mutex_unlock(&aitalk_wait_stop_mutex);

//*/
	}

	return 0;
}


//int ai_pipe[2];

extern sem_t sem_aitalk;
char *aitalk_pipe_buf = NULL;
int aitalk_pipe_put(char *data){
	if (!data){
		return -1;
	}

	free(aitalk_pipe_buf);
	aitalk_pipe_buf = NULL;
	aitalk_pipe_buf = strdup(data);
	sem_post(&sem_aitalk);
	return 0;
}

int aitalk_handler_wait(void){
	sem_wait(&sem_aitalk);
	return 0;
}	//*/


/*static int autoPlayIsDoing = false;
int mozart_auto_play_music(void){
	if (autoPlayIsDoing == true)
		return  0;
	char *url = NULL;
	url = ai_music_list_play_order(1);
	if (url)
		aitalk_pipe_put(aitalk_send_play_url(url));
	pr_debug("Auto play music...url = %s\n",url);
	autoPlayIsDoing = true;
	return 0;
}	//*/


int mozart_ai_domain_update(int domain){
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
		if (play_prompt){
			free(play_prompt);
			play_prompt = NULL;
		}
		play_prompt = strdup(prompt);
		pr_debug("\ndomain=%d,prompt=%s,play_prompt=%s\n",domain,prompt,play_prompt);
	}//*/
	pr_debug("\ndomain=%d,play_prompt=%s\n",domain,play_prompt);
	return 0;
}
/*
int mozart_tts(char *data,int enable_stop){

#if 1
	if (ai_tts(data) == -1){
		return -1;
	}
	if (enable_stop == true){
		music_info music;
		music.url ="/tmp/cloud_sync_record.mp3";
		music.artist = NULL;
		music.title = NULL;
		aitalk_pipe_put(aitalk_send_play_url(&music));
	}
	else{
		mozart_prompt_tone_key_sync("ai_cloud_sync", false);
	}
#else
	char *tts_url=NULL;
    tts_url=ai_output_url(data);
    //mozart_player_playurl(NULL,tts_url);
    mozart_prompt_tone_play_sync(tts_url,enable_stop);
    // __mozart_prompt_tone_play_sync(tts_url);
   // aitalk_pipe_put(tts_url);
    //aitalk_pipe_put(aitalk_send_play_url(tts_url));
	//ai_tts_time(data);
#endif
	return 0;
}
//*/
int mozart_vr_speech_interface_callback(vr_info *recog)
{
	int ret = 0;
//	int command = SDS_COMMAND_NULL;
//	char str[256]={0};
//	int vol =0;

/*	ret = mozart_license_flag_get();
	if (ret == -1){
		//char *str = "试用时间到啦，请更新授权，才能继续和我玩耍哦......";
		//mozart_tts(str,false);
		mozart_prompt_tone_key_sync("authority", false);
		goto exit_error;
	}	//*/

#if 0
	if ((recog->is_get_song_recommend    ==true)&&(recog->is_play_song_recommend    ==false)){
		recog->is_play_song_recommend   =true;
		aitalk_pipe_put(aitalk_send_play_music(NULL));
		aitalk_play_music = true;
	}

	switch(recog->status){
		case AIENGINE_STATUS_INIT:
			break;
	    case AIENGINE_STATUS_AEC:
	        break;
		case AIENGINE_STATUS_SEM:
			vol = mozart_volume_get();
			if (vol == 0){
				aitalk_pipe_put(aitalk_send_set_volume("10"));	//*/
				usleep(100000);
			}
			if (mozart_module_is_playing()==1){
				aitalk_pipe_put(aitalk_send_pause(NULL));
			}
			mozart_smartui_asr_start();
			mozart_prompt_tone_key_sync("welcome", false);
			break;
		case AIENGINE_STATUS_SEM:
			mozart_smartui_asr_recognize();
			break;
//		case AIENGINE_STATUS_TTS_ANSWER:
		case AIENGINE_STATUS_ASR_SUCCESS:
			mozart_ai_domain_update(recog->domain);
			switch(recog->domain){
			case RECOG_DOMAIN_CHAT:
				aitalk_play_music = false;
				if (recog->music.url){
				//	pr_debug("url=%s\n",recog->music.url);
					music_info music;
					music.url = recog->music.url;
					music.artist = recog->music.artist;
					music.title = recog->music.title;
					aitalk_pipe_put(aitalk_send_play_url(&music));
				}	//*/
				else{
					if (recog->output){
						if (recog->next_status == AIENGINE_STATUS_SEM){
							mozart_tts(recog->output,false);
							mozart_prompt_tone_key_sync("welcome", false);
						}
						else
							mozart_tts(recog->output,true);
					}
				}
				break;
			case RECOG_DOMAIN_MUSIC:
			case RECOG_DOMAIN_NETFM:
				if(RECOG_DOMAIN_MUSIC == recog->domain){
					aitalk_play_music = true;
				}
				else{
					aitalk_play_music = false;
				}

				if ((recog->music.url)&&(recog->music.title)){
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
					mozart_tts(str,false);
				}
				else if (recog->output){
					if (recog->next_status == AIENGINE_STATUS_SEM){
						mozart_tts(recog->output,false);
						mozart_prompt_tone_key_sync("welcome", false);
					}
					else
						mozart_tts(recog->output,true);
				}
				if (recog->music.url){
				//	pr_debug("url=%s\n",recog->music.url);
					music_info music;
					music.url = recog->music.url;
					music.artist = recog->music.artist;
					music.title = recog->music.title;
					aitalk_pipe_put(aitalk_send_play_url(&music));
				}	//*/
			/*	else{
					aitalk_pipe_put(aitalk_send_resume(NULL));
				}	//*/
				break;
			case RECOG_DOMAIN_CALENDAR:
			case RECOG_DOMAIN_WEATHER:
			case RECOG_DOMAIN_STOCK:
			case RECOG_DOMAIN_POETRY:
				aitalk_play_music = false;
				if (recog->output){
					if (recog->next_status == AIENGINE_STATUS_SEM){
						mozart_tts(recog->output,false);
						mozart_prompt_tone_key_sync("welcome", false);
					}
					else
						mozart_tts(recog->output,true);
				}
				break;
			case RECOG_DOMAIN_REMINDER:
				aitalk_play_music = false;
				break;
			case RECOG_DOMAIN_COMMAND:
				break;
			default:	//	unknow
				if (recog->output){
					if (recog->next_status == AIENGINE_STATUS_SEM){
						mozart_tts(recog->output,false);
						mozart_prompt_tone_key_sync("welcome", false);
					}
					else
						mozart_tts(recog->output,true);
				}
				else{
					//strcpy(str,"暂时不支持该领域指令......");
					//mozart_tts(str,false);
					mozart_prompt_tone_key_sync("error_invalid_domain", false);
				}
				break;
			}

			if((RECOG_DOMAIN_COMMAND == recog->domain)
			||(RECOG_DOMAIN_MUSIC== recog->domain)){
				if(recog->volume){
					command = SDS_COMMAND_VOLUME;
				}
				else{
					if(recog->operation){//---------------------------- operation -> command_type
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
					}
				}

				switch(command){
				case SDS_COMMAND_VOLUME:
					if (recog->volume){
						if(strcmp(recog->volume, "+") == 0){
							//strcpy(str,"为您增大音量...");
							mozart_prompt_tone_key_sync("volume_up", false);
						}
						else if(strcmp(recog->volume, "-") == 0){
						//	strcpy(str,"为您减小音量...");
							mozart_prompt_tone_key_sync("volume_down", false);
						}
						else if(strcmp(recog->volume, "max") == 0){
					//		strcpy(str,"为您设置最大音量");
							mozart_prompt_tone_key_sync("volume_max", false);
							free(recog->volume);
							recog->volume = strdup("100");
						}	//*/
						else if(strcmp(recog->volume, "min") == 0){
							//strcpy(str,"为您设置最小音量");
							mozart_prompt_tone_key_sync("volume_min", false);
							free(recog->volume);
							recog->volume = strdup("0");
						}	//*/
						else {
							strcpy(str,"为您设置音量为");
							strcat (str,recog->volume);
							mozart_tts(str,false);
						}
					//	mozart_tts(str,false);
						aitalk_pipe_put(aitalk_send_set_volume(recog->volume));
					}
					break;
				case SDS_COMMAND_MUSIC_PAUSE:
			//		strcpy(str,"暂停播放...");
			//		mozart_tts(str,false);
					mozart_prompt_tone_key_sync("pause", false);
					aitalk_play_music = false;
					if (mozart_module_is_playing()==1){
						aitalk_pipe_put(aitalk_send_pause(NULL));
					}
					//aitalk_pipe_put(aitalk_send_pause(NULL));
					break;
				case SDS_COMMAND_MUSIC_RESUME:
			//		strcpy(str,"继续播放...");
			//		mozart_tts(str,false);
					mozart_prompt_tone_key_sync("resume", false);
					aitalk_play_music = true;
					aitalk_pipe_put(aitalk_send_resume(NULL));
					break;
				case SDS_COMMAND_MUSIC_STOP:
			//		strcpy(str,"停止播放...");
			//		mozart_tts(str,false);
					aitalk_play_music = false;
					mozart_prompt_tone_key_sync("stop", false);
					aitalk_pipe_put(aitalk_send_stop_music(NULL));
					break;
				case SDS_COMMAND_MUSIC_PLAY:
					if ((recog->music.url == NULL)&&(recog->output== NULL)){
				//		strcpy(str,"播放...");
				//		mozart_tts(str,false);
						aitalk_play_music = true;
						mozart_prompt_tone_key_sync("resume", false);
						aitalk_pipe_put(aitalk_send_play_music(NULL));

					}
				//*/
					break;
				case SDS_COMMAND_MUSIC_PREVIOUS:
			//		strcpy(str,"上一首...");
			//		mozart_tts(str,false);
					mozart_prompt_tone_key_sync("previous", false);
					aitalk_pipe_put(aitalk_send_previous_music(NULL));
					break;
				case SDS_COMMAND_MUSIC_NEXT:
				//	strcpy(str,"下一首...");
				//	mozart_tts(str,false);
					mozart_prompt_tone_key_sync("next", false);
					aitalk_pipe_put(aitalk_send_next_music(NULL));
					break;
				case SDS_COMMAND_EXIT:
					//strcpy(str,"我走啦，有需要再叫我哦...");
					//mozart_tts(str,false);
					mozart_prompt_tone_key_sync("exit", false);
					break;
				default:
					if(RECOG_DOMAIN_COMMAND == recog->domain){
					//	strcpy(str,"暂时不支持此命令......");
					//	mozart_tts(str,false);

						mozart_prompt_tone_key_sync("error_invalid_domain", false);
					}
					break;
				}
			}
			case AIENGINE_STATUS_ERROR:
				switch (recog->error_type){
				case  AI_ERROR_ATUHORITY:
					mozart_smartui_asr_fail("授权失败");
					mozart_prompt_tone_key_sync("error_authority", false);
					break;
				case  AI_ERROR_INVALID_DOMAIN:
					mozart_smartui_asr_fail("不支持该领域指令");
					mozart_prompt_tone_key_sync("error_invalid_domain", false);
					break;
				case  AI_ERROR_SYSTEM:
					mozart_smartui_asr_fail("系统错误，请重新启动");
					mozart_prompt_tone_key_sync("error_system", false);
					break;
				case  AI_ERROR_NO_VOICE:
					mozart_smartui_asr_fail("您好像没说话哦请重新唤醒使用");
					mozart_prompt_tone_key_sync("error_no_voice", false);
					break;
				case  AI_ERROR_SERVER_BUSY:
					mozart_smartui_asr_fail("服务忙，请稍后再试");
					mozart_prompt_tone_key_sync("error_server_busy", false);
					break;
				case  AI_ERROR_NET_SLOW:
					mozart_smartui_asr_fail("网速有点慢，请稍后再试");
					mozart_prompt_tone_key_sync("error_net_slow", false);
					break;
				case  AI_ERROR_NET_FAIL:
					mozart_smartui_asr_fail("网络异常，请检测网络");
					mozart_prompt_tone_key_sync("error_net_fail", false);
					break;
				default:
					break;
				}

			break;
		default:
			break;
	}
#endif
	switch(recog->status){
		case AIENGINE_STATUS_INIT:
		case AIENGINE_STATUS_AEC: {
			//leds_set(LED_WHITE,255);
#if SUPPORT_SMARTUI
			mozart_smartui_asr_over();

			if(recog->domain == RECOG_DOMAIN_WEATHER) {
			//	mozart_smartui_weather_start(recog->weather);
				break;
			}
#endif
		//mozart_key_ignore_set(false);
	    	}
		break;

		case AIENGINE_STATUS_SEM: {
		//	leds_set(LED_GREEN,255);
		//	mozart_key_ignore_set(true);
#if SUPPORT_SMARTUI
			mozart_smartui_asr_start();
#endif
		}
		break;

		case AIENGINE_STATUS_PROCESS: {
		//	leds_set(LED_GREEN,255);
#if SUPPORT_SMARTUI
			mozart_smartui_asr_recognize();
#endif
		}
		break;

		default:
		break;
	}

//exit_error:
	if (ret == 0){
		if (__mozart_aitalk_network_state() == network_online){
			ret = 0;
		}
		else{
			ret = 1;
		}
	}
	return ret;
}


static void *aitalk_running_func(void *args)
{
	pthread_detach(pthread_self());
	static char *cmd;//[512]={}
	static const char *method;
	static json_object *c, *o;
	static bool is_valid = true;
	static struct aitalk_method *m;

	while (1) {
		int i;
		if (aitalk_handler_wait() == -1){
			pr_err("aitalk_handler_wait error!\n");
		}
		else//*/
		{
			cmd = aitalk_pipe_buf;
			if (cmd){
			//	pr_debug(">>>> %s: Recv: %s\n", __func__, cmd);

				c = json_tokener_parse(cmd);
				if (c){
					if (json_object_object_get_ex(c, "method", &o)){
						method = json_object_get_string(o);

						for (i = 0; i < ARRAY_SIZE(methods); i++) {
							m = &methods[i];
						//
							if (m->name){
								if (!strcmp(m->name, method)) {
									if (m->is_valid)
										is_valid = m->is_valid(c);	//*/
									if (is_valid){
										pr_debug("method = %s\n", m->name);
										m->handler(c);
									}
									else
										pr_debug("     %s invalid\n", cmd);
									break;
								}
							}
						}

						if (i >= ARRAY_SIZE(methods))
							pr_debug("invalid command: %s\n", method);

						json_object_put(c);
					}
				}
				else{
					pr_debug("[%s:%d] json_tokener_parse c error: %s\n", __func__,__LINE__, cmd);
				}
			/*	if (aitalk_pipe_buf){
					free(aitalk_pipe_buf);
					aitalk_pipe_buf = NULL;
				}	//*/
			}
		}
	}
	return NULL;
}

int aitalk_resume_player(void)
{
	int ret;

	send_player_state_change(player_play_state);
	ret = mozart_player_resume(aitalk_player_handler);
	if (ret)
		printf("[Warning] %s: mozart_player_resume fail\n", __func__);

	return ret;
}
int aitalk_pause_player(void)
{
	int ret;

	send_player_state_change(player_pause_state);
	ret = mozart_player_pause(aitalk_player_handler);
	if (ret)
		printf("[Warning] %s: mozart_player_pause fail\n", __func__);

	ret = mozart_player_wait_status(aitalk_player_handler, PLAYER_PAUSED, 500 * 1000);

	return ret;
}


int aitalk_startup(void)
{
//remove atalk
	pthread_t aitalk_thread;
	if (!aitalk_initialized) {
//remove atalk
/*
		if (hub_init())
			return -1;
//*/
		printf("%s:%d get_player_handler fail!\n", __func__,__LINE__);
		aitalk_player_handler =
			mozart_player_handler_get("aitalk", aitalk_player_status_callback, NULL);
		if (aitalk_player_handler == NULL) {
			printf("%s:%d get_player_handler fail!\n", __func__,__LINE__);
			return -1;
		}

		if (pthread_create(&aitalk_thread, NULL, aitalk_running_func, NULL) != 0) {
			printf("%s: Can't create aitalk_thread: %s\n",
			       __func__, strerror(errno));
			return -1;
		}
		pthread_detach(aitalk_thread);
//remove atalk
/*
		up_die = 0;
		list_init(&atalk.up_queue_list);
		if (pthread_create(&atalk.down_thread, NULL, atalk_update_queue_handle_func, NULL) != 0) {
			printf("%s: Can't create down_thread: %s\n",
				__func__, strerror(errno));
			return -1;
		}

		atalk.dp = dl_perform_init();
		if (!atalk.dp)
			return -1;
//*/
		aitalk_initialized = true;
	}
	#if (SUPPORT_VR == VR_ATALK)
		atalk_vendor_startup();
	#elif (SUPPORT_VR == VR_SPEECH)
		aitalk_vendor_startup();
	#endif
//remove atalk
//	atalk_vendor_startup();
	//aitalk_pipe_put(aitalk_send_play_music(NULL));
//	ai_set_enable(true);
	return 0;
}

int aitalk_shutdown(void)
{
//	pthread_mutex_lock(&aitalk_wait_stop_mutex);
//	pthread_cond_signal(&aitalk_wait_stop_cond);
//	pthread_mutex_unlock(&aitalk_wait_stop_mutex);
	pr_debug("---------444444444444-------------%s,%d\n",__func__,__LINE__);
/*	if (play_prompt){
		free(play_prompt);
		play_prompt = NULL;
	} //*/

	free(current_url);
	current_url = NULL;
	free(aitalk_pipe_buf);
	aitalk_pipe_buf = NULL;

	mozart_module_mutex_lock();
	__mozart_module_set_attach();
	__mozart_module_set_offline();
	__mozart_module_set_net();
	mozart_module_mutex_unlock();

	ai_set_enable(false);
	mozart_aitalk_do_stop();
/*	#if  SUPPORT_VR_SPEECH
	//	mozart_aitalk_cloudplayer_shutdown();
	#endif	//*/
//remove atalk
/*
	up_die = 1;
	dl_perform_stop(atalk.dp);

	pthread_mutex_lock(&cond_lock);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&cond_lock);

	pthread_join(atalk.down_thread, NULL);
	dl_perform_uninit(atalk.dp);

	atalk_vendor_shutdown();
//*/
	if (aitalk_player_handler) {
		mozart_player_handler_put(aitalk_player_handler);
		aitalk_player_handler = NULL;
	}
	aitalk_initialized = false;
	return 0;
}

