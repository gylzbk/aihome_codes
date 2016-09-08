/*
	dl_perform.h
 */
#ifndef __DL_PERFORM_H__
#define __DL_PERFORM_H__

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include <curl/curl.h>

typedef enum {
	DP_OK = 0,
	DP_STOPED,
	DP_ERROR,
	DP_ERR_RESUME,
	DP_TIMEOUT,
} DPres_t;

/**
 * struct dl_perform -
 */
struct dl_perform {
	pthread_t	pthread;
	CURL		*curlHnd;

	char		*url;
	FILE		*stream;

	long		current; /* unit: Bytes */
#define DL_CONNECT_TOUT_DEFAULT		6
	/* Timeout for http connection. 0, disable */
	long		connectTimeout;
#define DL_LOW_SPEED_LIMIT_DEFAULT	256
#define DL_LOW_SPEED_TIME_DEFAULT	20
	/* Timeout for speed lower than limit. 0, disable */
	long		lowSpeedLimit;
	long		lowSpeedTime;

	curl_off_t	maxSpeedLimit;

	enum {
		STOP,
		RUNNING,
	}		sw;

	void (*progressFunc)(int32_t dl_total, int32_t dl_now, void *userData);
	void *progressData;

	void (*endFunc)(DPres_t res, char *errStr, void *userData);
	void *endData;

	int		debugLevel;
};

/**
 * dl_perform_set_timeout -
 *
 * Note
 *	lowspeed and lowtimeout need set at same time.
 */
static inline void dl_perform_set_timeout(
	struct dl_perform *dp,
	long tout_connect,
	long lowspeed,
	long lowtimeout)
{
	if (tout_connect)
		dp->connectTimeout = tout_connect;

	if (lowspeed && lowtimeout) {
		dp->lowSpeedLimit = lowspeed;
		dp->lowSpeedTime = lowtimeout;
	}
}

/**
 * dl_perform_init -
 */
extern struct dl_perform *dl_perform_init(void);

/**
 * dl_perform_uninit -
 */
extern void dl_perform_uninit(struct dl_perform *dp);

/**
 * dl_perform_debug_set -
 */
extern void dl_perform_debug_set(int level);

/**
 * dl_perform_sync -
 */
extern int dl_perform_sync(
	struct dl_perform *dp,
	char *url,
	char *target,
	long offset);

/**
 * dl_perform_async -
 * @dp:
 * @url:
 * @target:
 * @offset
 */
extern int dl_perform_async(
	struct dl_perform *dp,
	char *url,
	char *target,
	long offset);

/**
 * dl_perform_stop -
 */
extern void dl_perform_stop(struct dl_perform *dp);

#endif /* __DL_PERFORM_H__ */
