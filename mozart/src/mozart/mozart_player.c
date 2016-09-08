#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "player_interface.h"

#include "mozart_player.h"

#ifndef MOZART_RELEASE
#define MOZART_PLAYER_DEBUG
#endif

#ifdef MOZART_PLAYER_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[PLAYER] %s: "fmt, __func__, ##args)
#else  /* MOZART_PLAYER_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_PLAYER_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[PLAYER] [Error] %s: "fmt, __func__, ##args)

int mozart_player_wait_status(player_handler_t *handler, player_status_t status, int timeout_us)
{
	unsigned long time_us;
	struct timeval start, end;
	player_status_t s = PLAYER_UNKNOWN;

	if (handler == NULL) {
		pr_err("handler is NULL\n");
		return -1;
	}

	gettimeofday(&start, NULL);

	while (1) {
		s = mozart_player_getstatus(handler);
		if (s == status)
			return 0;

		gettimeofday(&end, NULL);
		time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
		if (time_us > timeout_us)
			return -1;
		else
			usleep(50 * 1000);
	}

	return -1;
}

void mozart_player_force_stop(player_handler_t *handler)
{
	pr_debug("use force_stop!\n");
	mozart_player_forcestop(handler);
}

struct player_context mozart_player_force_pause(player_handler_t *handler)
{
	player_handler_t *h = NULL;
	struct player_context context;

	memset(&context, 0, sizeof(struct player_context));

	if (handler == NULL) {
		h = mozart_player_handler_get("player_saver", NULL, NULL);
		if (!h) {
			pr_err("player h get\n");
			return context;
		}
		handler = h;
	}

	context.uuid = mozart_player_getuuid(handler);
	context.url = mozart_player_geturl(handler);
	context.pos = mozart_player_getpos(handler);

	pr_debug("context.uuid: %s\n", context.uuid);
	pr_debug("context.url: %s\n", context.url);
	pr_debug("context.pos: %d\n", context.pos);

	mozart_player_force_stop(handler);

	if (context.uuid == NULL || context.url == NULL)
		pr_err("context is invalid!\n");

	if (h)
		mozart_player_handler_put(h);

	return context;
}

int mozart_player_force_resume(struct player_context context)
{
	player_handler_t *ori_handler = NULL;

	if (context.uuid == NULL || context.url == NULL) {
		pr_err("%s: context is invalid!\n", __func__);
		return -1;
	}

	ori_handler = mozart_player_handler_get(context.uuid, NULL, NULL);
	if (!ori_handler) {
		pr_err("player ori_handler get\n");
		return -1;
	}
	mozart_player_playurl(ori_handler, context.url);
	mozart_player_seek(ori_handler, context.pos);

	mozart_player_handler_put(ori_handler);

	return 0;
}
