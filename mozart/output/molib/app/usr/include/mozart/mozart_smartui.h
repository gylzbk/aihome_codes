#ifndef __MOZART_SMARTUI_H__
#define __MOZART_SMARTUI_H__

#include <stdbool.h>
#include "mozart_config.h"

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

extern void mozart_smartui_startup(void);
extern void mozart_smartui_shutdown(void);
extern void mozart_smartui_boot_start(void);
#if SUPPORT_BOARD==BOARD_DS1825
extern void mozart_smartui_boot_build_display(char *s);
#endif
#if SUPPORT_BOARD==board_wb38
extern void mozart_smartui_boot_build_display(char *s, int type);
#endif
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
extern void mozart_smartui_weather_start(char *area, char *weather, char*temperature, char *wind);
#endif

#endif	/* __MOZART_SMARTUI_H__ */
