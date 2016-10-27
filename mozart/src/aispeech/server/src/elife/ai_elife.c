
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include <json-c/json.h>

#include "ai_server.h"
#include "aiengine_app.h"


#include "ai_elife.h"
#include "ai_elife_send.h"


elife_t elife;

static char play_voice[256];
static char result[2048];
static char *cloud_pwd;

void _free(void){
	free(elife.input);
	elife.input = NULL;
	free(elife.type);
	elife.type = NULL;
	free(elife.sessionid);
	elife.sessionid = NULL;
}
void _resp_free(elife_resp_t *resp){
	free(resp->msg);
	resp->msg = NULL;
	free(resp->sessionid);
	resp->sessionid = NULL;
}

void _dev_free(elife_dev_t *dev){
	free(dev->cmd);
	dev->cmd = NULL;
	free(dev->name);
	dev->name = NULL;
	free(dev->position);
	dev->position = NULL;
}

void _scene_free(elife_scene_t *scene){
	free(scene->cmd);
	scene->cmd = NULL;
	free(scene->name);
	scene->name = NULL;
}

void _security_free(elife_security_t *security){
	free(security->cmd);
	security->cmd = NULL;
	free(security->name);
	security->name = NULL;
}


void _video_free(elife_video_t *video){
	free(video->cmd);
	video->cmd= NULL;
	free(video->name);
	video->name= NULL;
	free(video->director);
	video->director= NULL;
	free(video->actor);
	video->actor= NULL;
	free(video->area);
	video->area= NULL;
	free(video->v_type);
	video->v_type= NULL;
	free(video->episode_no);
	video->episode_no= NULL;
}


char  *_elife_get_recog(vr_info *recog){
	char *send_c = NULL;
	ai_elife_free();
	if (recog->input == NULL){
		return NULL;
	}
	elife.input = strdup(recog->input);
	switch(recog->domain){
		case RECOG_DOMAIN_MOVIE:
			elife.type = strdup("video");
			if (recog->operation){
				elife.video.cmd = strdup(recog->operation);
			}
			if(recog->movie.name){
				elife.video.name= strdup(recog->movie.name);
			}
			if(recog->movie.player){
				elife.video.actor= strdup(recog->movie.player);
			}
			if(recog->movie.area){
				elife.video.area= strdup(recog->movie.area);
			}
			if(recog->movie.type){
				elife.video.v_type= strdup(recog->movie.type);
			}
			if(recog->movie.director){
				elife.video.director= strdup(recog->movie.director);
			}
			if(recog->movie.sequence){
				elife.video.episode_no = strdup(recog->movie.sequence);
			}
			send_c = send_voice(&elife);
			break;
		case RECOG_DOMAIN_COMMAND:
			if (recog->scene){
				elife.type = strdup("scene");
				elife.scene.name = strdup(recog->scene);
				if (recog->operation){
					elife.scene.cmd = strdup(recog->operation);
				}
				send_c = send_scene(&elife);
			} else {
				elife.type = strdup("dev");
				if (recog->operation){
					elife.dev.cmd = strdup(recog->operation);
				}
				if (recog->device){
					elife.dev.name = strdup(recog->device);
				}
				if (recog->location){
					elife.dev.position = strdup(recog->location);
				}
				send_c = send_dev(&elife);
			}
			break;
	}

	printf("send ===>> %s \n",send_c);
	printf("input: %s\n", elife.input);
	printf("type: %s\n", elife.type);
	printf("dev.name: %s\n", elife.dev.name);
	printf("dev.cmd: %s\n", elife.dev.cmd);
	printf("dev.position: %s\n", elife.dev.position);
	printf("video.name: %s\n", elife.video.name);
	printf("video.cmd: %s\n", elife.video.cmd);
	printf("video.director: %s\n", elife.video.director);
	printf("video.actor: %s\n", elife.video.actor);
	printf("video.area: %s\n", elife.video.area);
	printf("video.v_type: %s\n", elife.video.v_type);
	printf("video.episode_no: %s\n", elife.video.episode_no);

	return send_c;
}


/******************************************************/
/**  API ***/
/******************************************************/
void ai_elife_init(void){
	cloud_pwd = NULL;
	elife.input = NULL;
	elife.type = NULL;
	elife.sessionid = NULL;
//------------------------------ resp
	elife.resp.msg = NULL;
	elife.resp.sessionid= NULL;
//------------------------------ voice
	elife.video.name = NULL;
	elife.video.cmd = NULL;
	elife.video.episode_no= NULL;
	elife.video.director= NULL;
	elife.video.v_type = NULL;
	elife.video.actor= NULL;
	elife.video.area= NULL;

//------------------------------ dev
	elife.dev.name = NULL;
	elife.dev.cmd = NULL;
	elife.dev.position = NULL;

//------------------------------ scene
	elife.scene.name = NULL;
	elife.scene.cmd = NULL;

//------------------------------ security
	elife.security.name = NULL;
	elife.security.cmd = NULL;
}

void ai_elife_free(void){
	_free();
	_video_free(&elife.video);
	_dev_free(&elife.dev);
	_scene_free(&elife.scene);
	_security_free(&elife.security);
	_resp_free(&elife.resp);
}

char *ai_elife_server(vr_info *recog){
	int ret = 0;
	char *send_c = NULL;
	json_object *result_j = NULL;
	json_object *ret_j = NULL;
	json_object *voice_resp_j = NULL;
	json_object *msg_j = NULL;
	json_object *is_mult_j = NULL;
	json_object *is_cy_j = NULL;
	json_object *sessionid_j = NULL;

	send_c = _elife_get_recog(recog);
	printf("elife send buf:       %s \n",send_c);
	memset(result, 0, sizeof(result));
	ret = send_wise_voice_cmd(send_c,result);

	DEBUG("result = %s\n",result);
	result_j = json_tokener_parse(result);
	if (result_j == NULL){
		ret = -1;
		PERROR("result_j = NULL\n");
		goto exit_error;
	}	//*/
	if (json_object_object_get_ex(result_j, "ret", &ret_j)){
		elife.resp.ret =(int)json_object_get_int(ret_j);
	}
	if (json_object_object_get_ex(result_j, "voice_resp", &voice_resp_j)){
		if (json_object_object_get_ex(voice_resp_j, "is_mult", &is_mult_j)){
			elife.resp.is_mult = (bool )json_object_get_boolean(is_mult_j);
		}
		if (json_object_object_get_ex(voice_resp_j, "is_cy", &is_cy_j)){
			elife.resp.is_cy= (bool )json_object_get_boolean(is_cy_j);
		}
		if (json_object_object_get_ex(voice_resp_j, "msg", &msg_j)){
			free(elife.resp.msg);
			elife.resp.msg = (char *)json_object_get_string(msg_j);
		}
		if (json_object_object_get_ex(voice_resp_j, "sessionid", &sessionid_j)){
			free(elife.resp.sessionid);
			elife.resp.sessionid = (char *)json_object_get_string(sessionid_j);
		}
	}


	printf("ret: %d\n", elife.resp.ret);
	printf("ret: %d\n", elife.resp.is_cy);
	printf("ret: %d\n", elife.resp.is_mult);
	printf("msg: %s\n", elife.resp.msg);
	printf("sessionid: %s\n", elife.resp.sessionid);

exit_error:
	DEBUG("--------------------------------------- ret = %d\n",ret);
	return elife.resp.msg;
}


