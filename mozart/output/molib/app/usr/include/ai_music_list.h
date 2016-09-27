#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "vr-speech_interface.h"
#include "ai_slot.h"

#if 0

#define MUSIC_LIST_MAX 10
#define MUSIC_LIST_SAVE_URL 0

typedef struct ai_music_list_t{
	music_info music[MUSIC_LIST_MAX];			//	music
	int music_num;				//	number
	int play_order;				//	order
}ai_music_list_t;

extern ai_music_list_t music_list;
extern music_info *ai_music_list_play_order(int order);
extern int ai_music_list_init(void);
extern int ai_music_list_free(void);
extern int ai_music_list_add_music(music_info *music);
#endif
