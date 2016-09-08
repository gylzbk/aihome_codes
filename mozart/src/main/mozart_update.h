/*
	mozart_update.h
 */
#ifndef __MOZART_UPDATE_H__
#define __MOZART_UPDATE_H__

#include <stdint.h>

/**
 * mozart_update_check_start -
 * @newer_callback:
 * @priv:
 * @interval_time:	Check inverval time. Unit: second
 */
extern int mozart_update_check_start(
	void (*newer_callback)(uint32_t, void *),
	void *priv,
	long interval_time);

extern int mozart_update_check_stop(void);

extern int mozart_update_active(void);

extern int mozart_update_cancel(void);

#endif /* __MOZART_UPDATE_H__ */
