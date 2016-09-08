#ifndef __ATALK_CONTROL_H__
#define __ATALK_CONTROL_H__

extern int start_atalk_app(void);
extern void stop_atalk_app(void);
extern int mozart_atalk_volume_set(int vol);
extern int mozart_atalk_prev(void);
extern int mozart_atalk_next(void);
extern int mozart_atalk_next_channel(void);
extern int mozart_atalk_toggle(void);
extern int mozart_atalk_pause(void);
extern int mozart_atalk_is_control_mode(void);
extern int mozart_atalk_is_online(void);
extern int mozart_atalk_switch_mode(void);
extern int mozart_atalk_love_audio(void);
extern int mozart_atalk_start(void);
extern int mozart_atalk_stop(void);
extern int mozart_atalk_stop_play(void);
extern int atalk_network_trigger(bool on);

#endif	/* __ATALK_CONTROL_H__ */
