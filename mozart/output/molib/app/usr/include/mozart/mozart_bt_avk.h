#ifndef __MOZART_BT_AVK_H__
#define __MOZART_BT_AVK_H__

extern bool __mozart_bt_avk_is_start(void);
extern int mozart_bt_avk_start(bool in_lock);
extern int mozart_bt_avk_do_play(void);
extern int mozart_bt_avk_do_pause(void);
extern int mozart_bt_avk_startup(void);
extern int mozart_bt_avk_shutdown(void);

#endif /* __MOZART_BT_AVK_H__ */
