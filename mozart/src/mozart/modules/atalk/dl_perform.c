/*
	dl_perform.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "dl_perform.h"

static int dl_debuglev = 0;

static void dl_debug(int level, char *format, ...)
{
	va_list args;

	if (level > dl_debuglev)
		return;

	printf("Download DEBUG %d: ", level);
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

static void dl_info(char *format, ...)
{
	va_list args;
	printf("Download INFO: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

static void dl_error(char *format, ...)
{
	va_list args;
	printf("Download ERROR: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

/**
 * dl_perform_init -
 */
struct dl_perform *dl_perform_init(void)
{
	struct dl_perform *dp;
	CURLcode err;

	dp = malloc(sizeof(struct dl_perform));
	if (!dp) {
		dl_error("%s.Alloc dl perform: %s", __func__, strerror(errno));
		goto err_alloc;
	}

	memset(dp, 0, sizeof(struct dl_perform));

	dp->sw			= STOP;
	dp->debugLevel		= dl_debuglev;

	dp->lowSpeedLimit	= DL_LOW_SPEED_LIMIT_DEFAULT; /* unit, BytePerSecond */
	dp->lowSpeedTime	= DL_LOW_SPEED_TIME_DEFAULT; /* unit, Seconds */
	dp->connectTimeout	= DL_CONNECT_TOUT_DEFAULT; /* unit, Seconds */
	dp->maxSpeedLimit	= 0;

	err = curl_global_init(CURL_GLOBAL_ALL);
	if (err) {
		dl_error("%s.%s", __func__, curl_easy_strerror(err));
		goto err_curl_init;
	}

	return dp;

err_curl_init:
	free(dp);

err_alloc:

	return NULL;
}

/**
 * dl_perform_uninit -
 */
void dl_perform_uninit(struct dl_perform *dp)
{
	/* FIXME: can not use. curl_global_cleanup() */
	//curl_global_cleanup();

	if (dp)
		free(dp);
}

/**
 * dl_perform_debug_set -
 */
void dl_perform_debug_set(int level)
{
	if (level)
		dl_debuglev = level;
}

static size_t dl_write_callback(
	char *ptr,
	size_t size,
	size_t nmemb,
	void *userdata)
{
	struct dl_perform *dp = (struct dl_perform *)userdata;
	dl_debug(2, "%s.%d\n", __func__, __LINE__);
	return fwrite(ptr, size, nmemb, dp->stream);
}

static int dl_progress_callback(
	void *clientp,
	curl_off_t dltotal,
	curl_off_t dlnow,
	curl_off_t ultotal,
	curl_off_t ulnow)
{
	struct dl_perform *dp = (struct dl_perform *)clientp;

	dl_debug(2, "%s.%d\n", __func__, __LINE__);

	/* Ignore ultotal and ulnow in download.
	 * Also maybe header has no field name "content-length".
	 */

	if (dp->progressFunc && (dltotal || dlnow))
		dp->progressFunc(dltotal, dlnow, dp->progressData);

	if (dp->sw == STOP) {
		/* Stop curl perform by return non-zero */
		return 1;
	}

	return 0;
}

static int dl_debug_trace(
	CURL *handle,
	curl_infotype type,
	char *data,
	size_t size,
	void *userptr)
{
	switch (type) {
	case CURLINFO_TEXT:
		dl_info("%s.CURLINFO_TEXT", __func__);
		break;
	case CURLINFO_HEADER_IN:
		dl_info("%s.CURLINFO_HEADER_IN", __func__);
		break;
	case CURLINFO_HEADER_OUT:
		dl_info("%s.CURLINFO_HEADER_OUT", __func__);
		break;
	case CURLINFO_DATA_IN:
		dl_info("%s.CURLINFO_DATA_IN", __func__);
		break;
	case CURLINFO_DATA_OUT:
		dl_info("%s.CURLINFO_DATA_OUT", __func__);
		break;
	case CURLINFO_SSL_DATA_IN:
		dl_info("%s.CURLINFO_SSL_DATA_IN", __func__);
		break;
	case CURLINFO_SSL_DATA_OUT:
		dl_info("%s.CURLINFO_SSL_DATA_OUT", __func__);
		break;
	case CURLINFO_END:
		dl_info("%s.CURLINFO_END", __func__);
		break;
	}

	return 0;
}

static void *dl_perform_func(void *data)
{
	struct dl_perform *dp = (struct dl_perform *)data;
	char curlError[CURL_ERROR_SIZE] = {0};
	CURLcode eCode;
	DPres_t res;

	/* Init the curl session */
	dp->curlHnd = curl_easy_init();
	if (!dp->curlHnd) {
		dl_error("%s.curl easy init failed", __func__);
		return NULL;
	}

	/* Set URL to get here */
	curl_easy_setopt(dp->curlHnd, CURLOPT_URL, dp->url);
	/* Set seek offset, also can be 0 */
	curl_easy_setopt(dp->curlHnd, CURLOPT_RESUME_FROM, dp->current);

	/* Enable progress meter, default enable */
	curl_easy_setopt(dp->curlHnd, CURLOPT_NOPROGRESS, 0L);
	/* Set curl error string buffer */
	curl_easy_setopt(dp->curlHnd, CURLOPT_ERRORBUFFER, curlError);
	//?? curl_easy_setopt(dp->curlHnd, CURLOPT_BUFFERSIZE, /* cache size */);

	/* Set callback functions */
	curl_easy_setopt(dp->curlHnd, CURLOPT_WRITEFUNCTION, dl_write_callback);
	curl_easy_setopt(dp->curlHnd, CURLOPT_WRITEDATA, dp);
	curl_easy_setopt(dp->curlHnd, CURLOPT_XFERINFOFUNCTION, dl_progress_callback);
	curl_easy_setopt(dp->curlHnd, CURLOPT_XFERINFODATA, dp);
	curl_easy_setopt(dp->curlHnd, CURLOPT_DEBUGFUNCTION, dl_debug_trace);
	curl_easy_setopt(dp->curlHnd, CURLOPT_DEBUGDATA, dp);

	/* Set connect timeout */
	if (dp->connectTimeout)
		curl_easy_setopt(dp->curlHnd, CURLOPT_CONNECTTIMEOUT, dp->connectTimeout);

	/* Set transport quality threshold */
	if (dp->lowSpeedLimit && dp->lowSpeedTime) {
		curl_easy_setopt(dp->curlHnd, CURLOPT_LOW_SPEED_LIMIT, dp->lowSpeedLimit);
		curl_easy_setopt(dp->curlHnd, CURLOPT_LOW_SPEED_TIME, dp->lowSpeedTime);
	}

	if (dp->maxSpeedLimit)
		curl_easy_setopt(dp->curlHnd, CURLOPT_MAX_RECV_SPEED_LARGE, dp->maxSpeedLimit);

	if (dp->debugLevel > 1) {
		/* Switch on full protocol/debug output while debug mode */
		curl_easy_setopt(dp->curlHnd, CURLOPT_VERBOSE, 1L);
	}

	eCode = curl_easy_perform(dp->curlHnd);
	if (eCode) {
		res = DP_ERROR;

		switch (eCode) {
		case CURLE_ABORTED_BY_CALLBACK:
			res = DP_STOPED;
			break;

		case CURLE_RANGE_ERROR:
			/* Remote HTTP Accept-Range: none */
			res = DP_ERR_RESUME;
			break;

		case CURLE_OPERATION_TIMEDOUT:
			res = DP_TIMEOUT;
			break;

		default:
			dl_debug(1, "%s. '%s' [%d] %s", __func__, dp->url, eCode, curlError);
		}
	} else {
		/* CURLE_OK(0) */
		res = DP_OK;
	}

	curl_easy_cleanup(dp->curlHnd);

	dp->sw = STOP;

	if (dp->stream)
		fclose(dp->stream);

	/* Accompish notify */
	if (dp->endFunc)
		dp->endFunc(res, curlError, dp->endData);

	return NULL;
}

/**
 * dl_perform_sync -
 */
int dl_perform_sync(
	struct dl_perform *dp,
	char *url,
	char *target,
	long offset)
{
	FILE *fp;

	if (!url || !target) {
		/* error handle */
		return -1;
	}

	dl_debug(1, "%s. download url: %s\n", url);

	fp = fopen(target, "w+b");
	if (!fp) {
		dl_error("%s. open target: %s", __func__, strerror(errno));
		return -1;
	}

	fseek(fp, offset, SEEK_CUR);

	dp->url		= url;
	dp->stream	= fp;
	dp->current	= offset;
	dp->sw		= RUNNING;

	dl_perform_func((void *)dp);

	return 0;
}

/**
 * dl_perform_async -
 */
int dl_perform_async(
	struct dl_perform *dp,
	char *url,
	char *target,
	long offset)
{
	FILE *fp;
	int err;

	if (!url || !target) {
		/* error handle */
		return -1;
	}

	dl_debug(1, "%s. download url: %s\n", __func__, url);

	fp = fopen(target, "w+b");
	if (!fp) {
		dl_error("%s. open target: %s", __func__, strerror(errno));
		return -1;
	}

	fseek(fp, offset, SEEK_CUR);

	dp->url		= url;
	dp->stream	= fp;
	dp->current	= offset;
	dp->sw		= RUNNING;

	err = pthread_create(&dp->pthread, NULL, dl_perform_func, (void *)dp);
	if (err) {
		dl_error("%s.perform pthread: %s", __func__, strerror(errno));
		fclose(fp);
		return -1;
	}
	pthread_detach(dp->pthread);

	return 0;
}

/**
 * dl_perform_stop -
 */
void dl_perform_stop(struct dl_perform *dp)
{
	dp->sw = STOP;
}
