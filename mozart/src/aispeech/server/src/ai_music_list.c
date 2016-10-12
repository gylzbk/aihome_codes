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
#include "music_list.h"

extern music_obj *g_m;

music_info *ai_music_list_play_order(int order)
{
	static int times = 0;
	if (times == 0) {
		order = 0;
		times++;
	}

	music_info *music;
	switch (order) {
	case 0:
		music = music_cur_get(g_m);
		if (music == NULL)
			printf("[%s %s %d] no song\n", __FILE__, __func__, __LINE__);

		break;
	case 1:
		music = music_next_get(g_m);
		/*XXX: maybe tone tip*/
		if (music == NULL) {
			printf("[%s %s %d] no next song\n", __FILE__, __func__, __LINE__);
			music = ai_song_recommend_push();
			if (music == NULL)
				return NULL;
			music_info *tmp;
			music_info_alloc(&tmp, music->title, music->artist, music->url);
			music_list_insert(g_m, tmp);
		}
		break;
	case -1:
		music = music_prev_get(g_m);
		/*XXX: maybe tone tip*/
		if (music == NULL) {
			music = ai_song_recommend_push();
			if (music == NULL)
				return NULL;
			music_info *tmp;
			music_info_alloc(&tmp, music->title, music->artist, music->url);	
			music_list_insert_head(g_m, tmp);
			printf("[%s %s %d] no previous song\n", __FILE__, __func__, __LINE__);
		}
		break;
	default:
		music = NULL;
		printf("[%s %s %d] order error\n", __FILE__, __func__, __LINE__);
		break;
	}

	if (music == NULL) {
		music = ai_song_recommend_push();
		if (music == NULL)
			return NULL;
		music_info *tmp;
		music_info_alloc(&tmp, music->title, music->artist, music->url);
		music_list_insert(g_m, tmp);
		printf("[%s %s %d]\n", __FILE__, __func__, __LINE__);
	}
	return music;
}

int ai_play_music_order(int order)
{
	music_info *music = NULL;
	music = ai_music_list_play_order(order);
	//pr_debug("url %s\n",url);
	if ((music)&&(music->url)){
		ai_aitalk_send(aitalk_send_play_url(music));
		return 0;
	}

	return -1;
}
