#ifndef MUSIC_LIST
#define MUSIC_LIST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

typedef struct music_info_t {
	LIST list;
	char *title;
	char *artist;
	char *url;
} music_info;

typedef struct music_obj_t {
	music_info head;
	int max;
	int cur_num;
	music_info *cur_music;
	music_info *cur_tmp;
} music_obj;

int music_list_alloc(music_obj **obj, int max);
int music_list_insert(music_obj *obj, music_info *info);
int music_info_alloc(music_info **info, char *title, char *artist, char *url);
int music_list_destroy(music_obj **obj);
int music_list_insert_head(music_obj *obj, music_info *info);
music_info *music_cur_get(music_obj *obj);
music_info *music_next_get(music_obj *obj);
music_info *music_prev_get(music_obj *obj);
int music_cur_set(music_obj *obj, music_info *m);

#endif
