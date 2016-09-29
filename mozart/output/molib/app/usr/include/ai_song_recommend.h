#ifndef _AI_SONG_RECOMMEND_H_
#define _AI_SONG_RECOMMEND_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"
#include "vr-speech_interface.h"
//#define MUSIC_SERVER		    "d.api.aispeech.com/data/music?"

#define SONG_LIST_MAX 10
#define SONG_GET_ERROR_MAX 5

typedef enum song_recommend_type_e{
	SONG_RECOMMEND_TYPE_AUTO,
	SONG_RECOMMEND_TYPE_ARTIST
}song_recommend_type_e;
typedef struct ai_song_recommend_t {
	bool  is_success;
	bool  is_running;
	bool  is_update;
	bool  is_getting;
	bool  is_stop;
	bool  is_wait_callback;
//	int  error_count;
	int  song_number;
	int  geted_number;
	music_info song[SONG_LIST_MAX];
	char *search_artist;
	song_recommend_type_e type;					//	type
}ai_song_recommend_t;

extern ai_song_recommend_t ai_song_list;

extern void ai_song_recommend_init(void);
extern int ai_song_recommend_update(char *text);
extern int ai_song_recommend_auto(void);
extern music_info *ai_song_recommend_push(void);
extern void ai_song_recommend_free_all(void);
extern void ai_song_recommend_artist(char *artist);
#endif
