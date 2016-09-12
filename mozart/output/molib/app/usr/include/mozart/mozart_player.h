#ifndef __MOZART_PLAYER_H__
#define __MOZART_PLAYER_H__

struct player_context {
	char *uuid;
	char *url;
	int pos;
};

extern int mozart_player_wait_status(player_handler_t *handler, player_status_t status, int timeout_us);
extern void mozart_player_force_stop(player_handler_t *handler);
extern struct player_context mozart_player_force_pause(player_handler_t *handler);
extern int mozart_player_force_resume(struct player_context context);

#endif	/* __MOZART_PLAYER_H__ */
