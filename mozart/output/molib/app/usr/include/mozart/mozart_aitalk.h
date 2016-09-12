#ifndef __MOZART_AITALK_H__
#define __MOZART_AITALK_H__

enum atalk_network_state {
	network_config,
	network_online,
	network_offline,
};

extern enum atalk_network_state __mozart_aitalk_network_state(void);
extern int __mozart_aitalk_network_trigger(enum atalk_network_state cur,
					  enum atalk_network_state ori, bool force);
extern int __mozart_aitalk_switch_mode(bool mode);
extern void mozart_switch_aitalk_module(bool in_lock);
extern void mozart_aitalk_net_change(bool online);

/* localplayer */
extern bool __mozart_atalk_localplayer_is_start(void);
extern int mozart_atalk_localplayer_start(bool in_lock);
extern int mozart_atalk_localplayer_do_play(void);
extern int mozart_atalk_localplayer_startup(void);
extern int mozart_atalk_localplayer_shutdown(void);

/* cloudplayer */
extern int create_atalk_cloudplayer_monitor_pthread(void);
extern void atalk_cloudplayer_monitor_cancel(void);
extern void atalk_cloudplayer_monitor_module_cancel(void);
extern bool atalk_cloudplayer_monitor_is_valid(void);
extern bool atalk_cloudplayer_monitor_is_module_cancel(void);
/* asr */
extern void mozart_aitalk_cloudplayer_update_context(char *uuid, char *url);
extern bool __mozart_aitalk_cloudplayer_is_run(void);
extern bool __mozart_aitalk_cloudplayer_is_start(void);
extern int mozart_aitalk_cloudplayer_start(bool in_lock);
extern int mozart_aitalk_cloudplayer_do_play(void);
extern int mozart_aitalk_cloudplayer_do_resume(void);
extern int mozart_aitalk_cloudplayer_do_pause(void);
extern int mozart_aitalk_cloudplayer_do_resume_pause(void);
extern int mozart_aitalk_cloudplayer_do_stop(void);
extern int mozart_aitalk_cloudplayer_startup(void);
extern int mozart_aitalk_cloudplayer_shutdown(void);
extern int mozart_aitalk_asr_start(void);
extern int mozart_aitalk_asr_over(void);
extern int mozart_aitalk_asr_startup(void);
extern int mozart_aitalk_asr_shutdown(void);

#endif /* __MOZART_ATALK_H__ */
