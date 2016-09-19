#ifndef __MOZART_SMARTUI_H__
#define __MOZART_SMARTUI_H__

#include <stdbool.h>
#include "vr-speech_interface.h"
#include "mozart_config.h"

#if (SUPPORT_SMARTUI)
extern char *mozart_smartui_owner(void);
extern bool mozart_smartui_is_boot_view(void);
extern bool mozart_smartui_is_net_view(void);
extern bool mozart_smartui_is_atalk_view(void);
extern bool mozart_smartui_is_bt_view(void);
extern bool mozart_smartui_is_linein_view(void);

extern void mozart_smartui_volume_update(void);
extern void mozart_smartui_wifi_update(void);
extern void mozart_smartui_update_update(bool need);
extern void mozart_smartui_update_hide(bool hide);
extern void mozart_smartui_battery_update(int capacity, int online);

extern void inline mozart_smartui_startup(void);//{return;}
extern void mozart_smartui_shutdown(void);
extern void mozart_smartui_boot_start(void);
extern void mozart_smartui_boot_build_display(char *s);
extern void mozart_smartui_boot_display(char *s);
extern void mozart_smartui_boot_welcome(void);
extern void mozart_smartui_boot_local(void);
extern void mozart_smartui_boot_factory_reset(void);
extern void mozart_smartui_boot_power_off(char *reason);

extern void mozart_smartui_net_start(void);
extern void mozart_smartui_net_receive_success(void);
extern void mozart_smartui_net_success(void);
extern void mozart_smartui_net_fail(char *reason);

extern void mozart_smartui_atalk_prompt(char *prompt);
extern void mozart_smartui_atalk_play(char *vendor, char *title, char *artist, char *prompt);
extern void mozart_smartui_atalk_toggle(bool play);

extern void mozart_smartui_asr_start(void);
extern void mozart_smartui_asr_recognize(void);
extern void mozart_smartui_asr_offline(void);
extern void mozart_smartui_asr_success(char *s);
extern void mozart_smartui_asr_fail(char *s);
extern void mozart_smartui_asr_over(void);

extern void mozart_smartui_bt_start(void);
extern void mozart_smartui_bt_connecting(void);
extern void mozart_smartui_bt_connected(void);
extern void mozart_smartui_bt_disconnect(void);
extern void mozart_smartui_bt_hs(void);
extern void mozart_smartui_bt_hs_disconnect(void);
extern void mozart_smartui_bt_play(void);
extern int mozart_smartui_bt_get_play_col_num(void);
extern void mozart_smartui_bt_play_barview(int *data);
extern void mozart_smartui_bt_toggle(bool play);

extern void mozart_smartui_linein_play(void);

extern void mozart_smartui_update(void);
extern void mozart_smartui_update_start(void);
extern void mozart_smartui_update_progress(int percent);
extern void mozart_smartui_update_cancel(void);

#if SUPPORT_BOARD==BOARD_WB38
extern void mozart_smartui_weather_start(weather_info recog);
#endif

#else
extern char inline  *mozart_smartui_owner(void){
	return NULL;
}

extern bool   inline mozart_smartui_is_boot_view(void){
	return false;
}

extern bool   inline   mozart_smartui_is_net_view(void){
	return false;
}

extern bool   inline  mozart_smartui_is_atalk_view(void){
	return false;
}

extern bool   inline  mozart_smartui_is_bt_view(void){
	return false;
}

extern bool   inline  mozart_smartui_is_linein_view(void){
	return false;
}


extern void   inline   mozart_smartui_volume_update(void){
	return;
}

extern void   inline   mozart_smartui_wifi_update(void){
	return;
}

extern void   inline   mozart_smartui_update_update(bool need){
	return;
}

extern void   inline   mozart_smartui_update_hide(bool hide){
	return;
}

extern void   inline   mozart_smartui_battery_update(int capacity, int online){
	return;
}


extern void   inline   mozart_smartui_startup(void){
	return;
}

extern void   inline   mozart_smartui_shutdown(void){
	return;
}

extern void   inline   mozart_smartui_boot_start(void){
	return;
}

extern void   inline   mozart_smartui_boot_build_display(char *s){
	return;
}

extern void   inline   mozart_smartui_boot_display(char *s){
	return;
}

extern void   inline   mozart_smartui_boot_welcome(void){
	return;
}

extern void   inline   mozart_smartui_boot_local(void){
	return;
}

extern void   inline   mozart_smartui_boot_factory_reset(void){
	return;
}

extern void   inline   mozart_smartui_boot_power_off(char *reason){
	return;
}


extern void   inline   mozart_smartui_net_start(void){
	return;
}

extern void   inline   mozart_smartui_net_receive_success(void){
	return;
}

extern void   inline   mozart_smartui_net_success(void){
	return;
}

extern void   inline   mozart_smartui_net_fail(char *reason){
	return;
}


extern void   inline   mozart_smartui_atalk_prompt(char *prompt){
	return;
}

extern void   inline   mozart_smartui_atalk_play(char *vendor, char *title, char *artist, char *prompt){
	return;
}

extern void   inline   mozart_smartui_atalk_toggle(bool play){
	return;
}

extern void   inline   mozart_smartui_asr_start(void){
	return;
}

extern void   inline   mozart_smartui_asr_recognize(void){
	return;
}

extern void   inline   mozart_smartui_asr_offline(void){
	return;
}

extern void   inline   mozart_smartui_asr_success(char *s){
	return;
}

extern void   inline   mozart_smartui_asr_fail(char *s){
	return;
}

extern void   inline   mozart_smartui_asr_over(void){
	return;
}


extern void   inline   mozart_smartui_bt_start(void){
	return;
}

extern void   inline   mozart_smartui_bt_connecting(void){
	return;
}

extern void   inline   mozart_smartui_bt_connected(void){
	return;
}

extern void   inline   mozart_smartui_bt_disconnect(void){
	return;
}

extern void   inline   mozart_smartui_bt_hs(void){
	return;
}

extern void   inline   mozart_smartui_bt_hs_disconnect(void){
	return;
}

extern void   inline   mozart_smartui_bt_play(void){
	return;
}

extern int   inline   mozart_smartui_bt_get_play_col_num(void){
	return 0;
}

extern void   inline   mozart_smartui_bt_play_barview(int *data){
	return;
}

extern void   inline   mozart_smartui_bt_toggle(bool play){
	return ;
}


extern void   inline      mozart_smartui_linein_play(void){
	return;
}


extern void   inline       mozart_smartui_update(void){
	return;
}

extern void   inline     mozart_smartui_update_start(void){
	return;
}

extern void   inline      mozart_smartui_update_progress(int percent){
	return;
}

extern void   inline     mozart_smartui_update_cancel(void){
	return;
}
#endif

#endif	/* __MOZART_SMARTUI_H__ */
