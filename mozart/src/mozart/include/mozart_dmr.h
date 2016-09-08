#ifndef __MOZART_DMR_H__
#define __MOZART_DMR_H__

extern bool __mozart_dmr_is_start(void);
extern int mozart_dmr_start(bool in_lock);
extern int mozart_dmr_do_resume(void);
extern int mozart_dmr_do_pause(void);
extern int mozart_dmr_startup(void);
extern int mozart_dmr_shutdown(void);

#endif /* __MOZART_DMR_H__ */
