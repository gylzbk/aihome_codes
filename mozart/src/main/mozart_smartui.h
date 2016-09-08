#ifndef __MOZART_SMARTUI_H__
#define __MOZART_SMARTUI_H__

extern char *mozart_smartui_owner(void);
extern bool mozart_smartui_is_boot_view(void);
extern bool mozart_smartui_is_net_view(void);
extern bool mozart_smartui_is_atalk_view(void);
extern bool mozart_smartui_is_bt_view(void);
extern bool mozart_smartui_is_linein_view(void);

extern void mozart_smartui_volume_update(void);
extern void mozart_smartui_battery_update(void);
extern void mozart_smartui_startup(void);
extern void mozart_smartui_boot_link(void);
extern void mozart_smartui_boot_linked(void);
extern void mozart_smartui_boot_welcome(void);
extern void mozart_smartui_boot_local(void);
extern void mozart_smartui_net_start(void);
extern void mozart_smartui_net_success(void);
extern void mozart_smartui_net_fail(void);
extern void mozart_smartui_atalk_prompt(char *prompt);
extern void mozart_smartui_atalk_play(char *vendor, char *title, char *artist, char *prompt);
extern void mozart_smartui_atalk_toggle(bool play);
extern void mozart_smartui_asr_start(void);
extern void mozart_smartui_asr_offline(void);
extern void mozart_smartui_asr_success(void);
extern void mozart_smartui_asr_fail(int index);
extern void mozart_smartui_bt_play(void);
extern void mozart_smartui_bt_toggle(bool play);
extern void mozart_smartui_linein_play(void);

#endif	/* __MOZART_SMARTUI_H__ */
