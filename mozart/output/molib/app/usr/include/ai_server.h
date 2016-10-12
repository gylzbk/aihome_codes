#ifndef __AI_SERVER_H__
#define __AI_SERVER_H__

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
#include "aiengine.h"
#include "ai_song_list.h"
//#include "ai_music_list.h"
//#include "ai_tts_msg.h"
//#include "ai_player.h"
//#include "ai_radio.h"
//#include "ai_music.h"
//#include "ai_curl.h"
//#include "ai_addr.h"
#include "ai_zlog.h"
#include "cJSON.h"

#define APPKEY 				"146337845885959a"
#define SERKEY 				"dbc0313b7467d669ce2f5148ea992bf8"
extern int ai_server_fun(vr_info *recog);
extern int ai_server_init(void);
extern int ai_server_restart(void);
extern music_info *ai_music_list_play_order(int order);
extern ai_sem_search_t ai_sem_search;

#endif
