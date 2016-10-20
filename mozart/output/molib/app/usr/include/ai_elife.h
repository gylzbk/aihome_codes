
#ifndef _AI_ELIFE_H_
#define _AI_ELIFE_H_

#include "elife_doss.h"

typedef struct elife_dev_t {
	char *name;
	char *position;
	char *cmd;
}elife_dev_t;

typedef struct elife_video_t {
	char *name;
	char *cmd;
	char *director;
	char *actor;
	char *v_type;
	char *area;
	char *episode_no;
}elife_video_t;

typedef struct elife_security_t {
	char *name;
	char *cmd;
}elife_security_t;

typedef struct elife_scene_t {
	char *name;
	char *cmd;
}elife_scene_t;

typedef struct elife_resp_t {
	int ret;
	bool is_mult;
	bool is_cy;
	char *msg;
	char *sessionid;
}elife_resp_t;

typedef struct elife_t{
	char *type;
	char *input;
	char *sessionid;
	elife_video_t video;
	elife_dev_t dev;
	elife_security_t security;
	elife_scene_t scene;
	elife_resp_t resp;
}elife_t;	//*/

extern elife_t elife;

extern void ai_elife_init(void);
extern void ai_elife_free(void);
extern char *ai_elife_server(vr_info *recog);

#endif
