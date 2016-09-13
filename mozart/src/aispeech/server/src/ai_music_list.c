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
#include <dirent.h>
#include <netinet/in.h>
#include <linux/soundcard.h>
#include "ai_music_list.h"
#include "ai_slot.h"
#include "cJSON.h"

#include "ai_song_recommend.h"


#define MUSIC_LIST_PATH "/usr/data/music/"

#define MUSIC_ONLINE_LIST "/tmp/play.list"

cJSON * pJsonListLast = NULL;

ai_music_list_t music_list;

int music_num_old = 0;

int music_num_new = 0;
extern bool aitalk_play_music;
int ai_music_list_get_number(void){
	return music_list.music_num;
}
/*
int ai_music_list_delect(void){
	char sys_cmd[100];
	sprintf(sys_cmd, "rm -f %s",MUSIC_ONLINE_LIST);
	system(sys_cmd);
	music_list.music_num =0;
	return 0;
}//*/


int ai_music_list_init(void){
	int i =0;
	music_list.music_num = 0;
	music_list.play_order =0;		//current play order
	for (i=0;i<MUSIC_LIST_MAX;i++){
		music_list.music[i].url = NULL;
		music_list.music[i].artist = NULL;
		music_list.music[i].title= NULL;
	}
	return 0;
}

//	----------------------------------  释放预分配的内存
int ai_music_list_free(void){
	int i =0;

	for (i=0;i<MUSIC_LIST_MAX;i++){
		free(music_list.music[i].url);
		music_list.music[i].url = NULL;
		free(music_list.music[i].artist);
		music_list.music[i].artist = NULL;
		free(music_list.music[i].title);
		music_list.music[i].title= NULL;
	}
	music_list.play_order = 0;
	music_list.music_num =0;

	return 0;
}

int ai_music_list_add_music(music_info *music)
{
    int error = 0;
	int i = 0;
	if((music->url == NULL)){
		PERROR("Error: music->url = null\n");
		error = -1;
		goto exit_error;
	}
	//---------------------list is not full ,add to end
	if (music_list.music_num < MUSIC_LIST_MAX){
		free(music_list.music[music_list.music_num].url);
		music_list.music[music_list.music_num].url =NULL;
		free(music_list.music[music_list.music_num].artist);
		music_list.music[music_list.music_num].artist =NULL;
		free(music_list.music[music_list.music_num].title);
		music_list.music[music_list.music_num].title =NULL;
		if (music->url){
			music_list.music[music_list.music_num].url = strdup(music->url);
		}
		if (music->artist){
			music_list.music[music_list.music_num].artist = strdup(music->artist);
		}
		if (music->title){
			music_list.music[music_list.music_num].title = strdup(music->title);
		}
		music_list.music_num ++;
	}
	//--------------------list is full ,clean hand and  add to end
	else{
		free(music_list.music[0].url);
		music_list.music[0].url = NULL;
		free(music_list.music[0].artist);
		music_list.music[0].artist = NULL;
		free(music_list.music[0].title);
		music_list.music[0].title= NULL;
		for (i=1;i<music_list.music_num;i++){
			music_list.music[i-1].url= music_list.music[i].url;
			music_list.music[i-1].artist= music_list.music[i].artist;
			music_list.music[i-1].title= music_list.music[i].title;
		}
		//--------------- copy music info to list end.
		free(music_list.music[music_list.music_num-1].url);
		music_list.music[music_list.music_num-1].url = NULL;
		free(music_list.music[music_list.music_num-1].artist);
		music_list.music[music_list.music_num-1].artist = NULL;
		free(music_list.music[music_list.music_num-1].title);
		music_list.music[music_list.music_num-1].title = NULL;
		if (music->url){
			music_list.music[music_list.music_num-1].url = strdup(music->url);
		}
		if (music->artist){
			music_list.music[music_list.music_num-1].artist = strdup(music->artist);
		}
		if (music->title){
			music_list.music[music_list.music_num-1].title = strdup(music->title);
		}
	}
	#if 0
	DEBUG("music_num = %d\n",music_list.music_num);
	for (i=0;i<music_list.music_num;i++){
		DEBUG("%2d : url = %s\n",i,music_list.music[i].url);
		DEBUG("artist = %s\n",music_list.music[i].artist);
		DEBUG("title = %s\n",music_list.music[i].title);
	}
	#endif
exit_error:
	if (error == -1){
		//ai_music_list_free();
	}
	return error;
}

music_info *ai_music_list_play_order(int order){
//	music_info *music,
	int play_order = 0;
	play_order = music_list.play_order + order;
	if (play_order < 0){
		play_order = 0;
	}
	if (play_order >= music_list.music_num){
		DEBUG("At the end of music list.\n");//	get new song from list
		music_info *music;
		music = ai_song_recommend_push();
		if (music->url != NULL){
			ai_music_list_add_music(music);
		}
		else{
		//	mozart_prompt_tone_key_sync("error_net_fail", true);
			return NULL;
		}
	}
	if (play_order >= MUSIC_LIST_MAX){
		play_order = MUSIC_LIST_MAX - 1;
	}

	music_list.play_order = play_order;

	if(music_list.play_order >1){
		play_order = music_list.play_order -1;
	}
	else{
		play_order = 0;
	}
	DEBUG("play_order = %d\n",play_order);
	return &music_list.music[play_order];
}


//bool play_error_tone = false;
int ai_play_music_order(int order){
	music_info *music = NULL;
	music = ai_music_list_play_order(order);
//	pr_debug("url %s\n",url);
	if ((music)&&(music->url)){
		ai_aitalk_send(aitalk_send_play_url(music));
	//	play_error_tone = false;
		return 0;
	}

	aitalk_play_music = false;
/*	if (play_error_tone == false){
		play_error_tone = true;
		mozart_prompt_tone_key_sync("error_net_fail", false);
	}	//*/

	return -1;
}



#if 0

//---------------------------------------- Play Previous
char *ai_musicListPlayPrevious(void){
	int error = 0;
	static char *url = NULL;
	int num = ai_music_list_get_number();
	if (num <= 0){
		error = -1;
		PERROR("Song list is empty...\n");
		goto exit_error;
	}

	if(music_list.play_order <= 0)
		music_list.play_order = num;
	music_list.play_order --;

	ai_music_list_play_order(music_list.play_order);
	url = music_list.play_url;
	DEBUG("num = %d,order = %d..\n",num,music_list.play_order);
	DEBUG("url = %s...\n",url);
exit_error:
	if (error){
		url = NULL;
	}
	return url;
}

//---------------------------------------- Play Next
char *ai_musicListPlayNext(void){
	int error = 0;
	static char *url = NULL;
	int num = ai_music_list_get_number();
	if (num <= 0){
		error = -1;
		PERROR("Song list is empty...\n");
		goto exit_error;
	}
	music_list.play_order ++;
	if(music_list.play_order >= num)
		music_list.play_order = 0;

	ai_music_list_play_order(music_list.play_order);

	url = music_list.play_url;
	DEBUG("num = %d,order = %d..\n",num,music_list.play_order);
	DEBUG("url = %s...\n",url);
exit_error:
	if (error){
		url = NULL;
	}
	return url;
}

#endif
