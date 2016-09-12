#ifndef __MOZART_STATUS_H__
#define __MOZART_STATUS_H__

enum sleep_state {
	sleep_idle = 0,
	sleep_wait,
	sleep_start,
	sleep_invalid,
};

extern void mozart_sleep_service_startup(void);
#endif /* __MOZART_STATUS_H__ */

