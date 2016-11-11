#ifndef _AI_SONG_LIST_H_
#define _AI_SONG_LIST_H_

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

#define SONG_LIST_MAX 11
#define SONG_GET_ERROR_MAX 2
#define SONG_GET_WAIT_S 120

typedef enum song_type_e{
	SONG_TYPE_AUTO,
	SONG_TYPE_ARTIST
}song_type_e;


typedef struct ai_song_list_t {
	bool  is_init;
	bool  is_running;
	bool  is_working;
	bool  is_set_renew;
	bool  is_getting;
	bool  is_success;
	bool  set_send_music;
	bool  is_send_music;
	bool is_start_update;
	bool  is_stop;
	bool  is_wait_callback;
	bool is_list_end;
	song_type_e type;					//	type
	char *artist;
	int renew_delay_s;
//----------------------- songs
	int song_num;
	int push_num;
	music_info song[SONG_LIST_MAX];
}ai_song_list_t;

extern ai_song_list_t ai_song_list;
extern int ai_song_list_get_from_param(cJSON *param);
extern int ai_song_list_init(void);
extern void ai_song_list_exit(void);
extern void ai_song_list_renew_artist(char *artist);
extern void ai_song_list_renew_auto(void);
extern music_info *ai_song_list_push(void);
extern int ai_song_list_set_enable(bool enable);

#endif


