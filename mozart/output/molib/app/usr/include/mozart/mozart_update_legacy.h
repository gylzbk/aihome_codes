/*
	mozart_update_legacy.h
 */
#ifndef __MOZART_UPDATE_LEGACY_H__
#define __MOZART_UPDATE_LEGACY_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
	UNRECOVERABLE_ERR = -1,
	NEW_VERSION_OK = 0,
	NEW_VERSION_ABORT = 1,
} ResultCallBack_t;

/**
 * mozart_update_check_start -
 * @newer_callback:
 * @priv:
 * @interval_time:	Check inverval time. Unit: second
 *
 * Note:
 * 	ResultCallBack_t (*newer_callback)(float, void *)
 * 	return UNRECOVERABLE_ERR, Unrecoverable error happen
 * 		and quit check.
 * 	return NEW_VERSION_OK, New version is valid and mark
 * 		update flag.
 * 	retudn NEW_VERSION_ABORT, Abort this version and continue
 * 		check remote update.
 */
extern int mozart_update_check_start(
	ResultCallBack_t (*newer_callback)(uint32_t, void *),
	void *priv,
	long interval_time);

extern int mozart_update_check_stop(void);

/**
 * mozart_update_start -
 * @perform_callback:
 * @priv:
 * @tsize_callback:	args total_size unit: Kbytes.
 * @tsize_priv:
 */
extern int mozart_update_start(
	void (*perform_callback)(uint32_t, void *),
	void *per_priv,
	void (*tsize_callback)(char *, int, void *),
	void *tsize_priv);

extern int mozart_update_cancel(void);

extern int mozart_direct_update(void);

extern int mozart_is_backfrom_update(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOZART_UPDATE_LEGACY_H__ */
