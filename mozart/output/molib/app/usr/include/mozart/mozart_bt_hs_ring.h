#ifndef __MOZART_BT_HS_RING_H__
#define __MOZART_BT_HS_RING_H__

extern bool __mozart_bt_hs_ring_is_start(void);
extern int mozart_bt_hs_ring_start(bool in_lock);
extern int mozart_bt_hs_ring_stop(bool in_lock);
extern int mozart_bt_hs_ring_startup(void);
extern int mozart_bt_hs_ring_shutdown(void);

#endif /* __MOZART_BT_HS_RING_H__ */
