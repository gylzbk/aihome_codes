#ifndef __MOZART_UPDATE_CONTROL_H__
#define __MOZART_UPDATE_CONTROL_H__

extern int mozart_update_control_try_start(bool in_lock);

extern void mozart_update_control_backfrom_update(void);

extern int mozart_update_control_startup(void);

extern int mozart_update_control_shutdown(void);
#endif /* __MOZART_UPDATE_CONTROL_H__ */
