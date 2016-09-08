#ifndef __MOZART_BT_HS_H__
#define __MOZART_BT_HS_H__

extern bool __mozart_bt_hs_is_start(void);
extern int mozart_bt_hs_start(bool in_lock);
extern int mozart_bt_hs_stop(bool in_lock);
extern int mozart_bt_hs_startup(void);
extern int mozart_bt_hs_shutdown(void);

#endif /* __MOZART_BT_HS_H__ */
