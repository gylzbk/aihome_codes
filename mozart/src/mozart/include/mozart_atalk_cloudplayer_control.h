#ifndef __MOZART_ATALK_CONTROL_H__
#define __MOZART_ATALK_CONTROL_H__

enum wifi_state {
	wifi_none = -1,
	wifi_start,
	wifi_end_ok,
	wifi_end_fail,
};

extern int atalk_cloudplayer_resume_player(void);
extern int atalk_cloudplayer_pause_player(void);
extern int atalk_cloudplayer_stop_player(void);

extern int atalk_cloudplayer_send_wifi_state(enum wifi_state state);
extern int __atalk_switch_mode(bool attach);
extern int atalk_cloudplayer_volume_change(int vol);
extern int atalk_cloudplayer_volume_set(int vol);
extern int atalk_cloudplayer_previous_music(void);
extern int atalk_cloudplayer_next_music(void);
extern int atalk_next_channel(void);

extern int atalk_love_audio(void);
extern void atalk_vendor_startup(void);
extern void atalk_vendor_shutdown(void);
extern int atalk_cloudplayer_startup(void);
extern int atalk_cloudplayer_shutdown(void);

#endif /* __MOZART_ATALK_CONTROL_H__ */
