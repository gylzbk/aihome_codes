#ifndef __KEY_FUNCTION_H__
#define __KEY_FUNCTION_H__

#include "mozart_config.h"

typedef enum snd_source_t{
    SND_SRC_NONE = -1,
    SND_SRC_ATALK, /* atalk music */
#if (SUPPORT_BT == BT_BCM)
    SND_SRC_BT_AVK,
#endif
    SND_SRC_LINEIN,
    SND_SRC_MAX,
} snd_source_t;

extern snd_source_t snd_source;
extern char *keyvalue_str[];
extern char *keycode_str[];
extern int tfcard_status;

extern char *app_name;
extern snd_source_t snd_source;
extern void mozart_stop_snd_source_play(void);
extern void mozart_wifi_mode(void);
extern void mozart_config_wifi(void);
extern void mozart_previous_song(void);
extern void mozart_next_song(void);
extern void mozart_play_pause(void);
extern void mozart_music_favorite(void);
extern void mozart_volume_up(void);
extern void mozart_volume_down(void);
extern void mozart_linein_on(void);
extern void mozart_linein_off(void);
extern void mozart_snd_source_switch(void);
extern void mozart_music_list(int);
extern void mozart_factory_reset(void);

#endif
