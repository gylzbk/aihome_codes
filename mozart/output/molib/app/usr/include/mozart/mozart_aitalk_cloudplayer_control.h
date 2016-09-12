#ifndef __MOZART_AITALK_CONTROL_H__
#define __MOZART_AITALK_CONTROL_H__

#include "aiengine_app.h"
#include "mozart_aitalk_asr.h"

enum wifi_state {
	wifi_none = -1,
	wifi_start,
	wifi_end_ok,
	wifi_end_fail,
};

extern int aitalk_cloudplayer_resume_player(void);
extern int aitalk_cloudplayer_pause_player(void);
extern int aitalk_cloudplayer_stop_player(void);

extern int aitalk_cloudplayer_send_wifi_state(enum wifi_state state);
extern int __aitalk_switch_mode(bool attach);
extern int aitalk_cloudplayer_volume_change(int vol);
extern int aitalk_cloudplayer_volume_set(int vol);
extern int aitalk_cloudplayer_previous_music(void);
extern int aitalk_cloudplayer_next_music(void);
extern int aitalk_next_channel(void);

extern int aitalk_love_audio(void);
extern void aitalk_vendor_startup(void);
extern void aitalk_vendor_shutdown(void);
extern int aitalk_cloudplayer_startup(void);
extern int aitalk_cloudplayer_shutdown(void);

extern int mozart_aitalk_cloudplayer_do_resume_pause(void);


extern int mozart_vr_speech_interface_callback(vr_info *recog);
#endif /* __MOZART_ATALK_CONTROL_H__ */

