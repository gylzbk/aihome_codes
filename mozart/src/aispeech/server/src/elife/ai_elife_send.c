#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <semaphore.h>
#include <json-c/json.h>
#include "vr-speech_interface.h"
#include "aiengine_app.h"

#include "ai_elife.h"

static char *pjson = NULL;

char *_send_obj(elife_t *elife, json_object *obj)
{
	json_object *o = NULL;
	const char *s = NULL;
	free(pjson);
	pjson =   NULL;

	if((elife == NULL)
	||(elife->input == NULL)
	||(elife->type == NULL)
	||(obj == NULL)){
		if (obj){
			json_object_put(obj);
			obj = NULL;
		}
		goto exit_err;
	}

	o = json_object_new_object();
	if (o == NULL){
		goto exit_err;
	}
	/* notification */
	json_object_object_add(o, "input", json_object_new_string(elife->input));
	json_object_object_add(o, "type", json_object_new_string(elife->type));
	if (elife->sessionid){
		json_object_object_add(o, "sessionid", json_object_new_string(elife->sessionid));
	}
	json_object_object_add(o, "params", obj);
	s = json_object_to_json_string(o);
	if (s){
		pjson = strdup(s);
	}
exit_err:
	if (o){
		json_object_put(o);
	}
	else {
		if (obj){
			json_object_put(obj);
		}
	}
	return pjson;
}


//---------------------------------------------------- dev
char *send_dev(elife_t *elife){
	DEBUG("++++++++++++++++++++++ send_dev \n");
	json_object *params = NULL;

	params = json_object_new_object();
	if (params == NULL){
		DEBUG("-------------------------- params error\n");
		return NULL;
	}
	if(elife->dev.name){
		json_object_object_add(params, "name", json_object_new_string(elife->dev.name));
	}
	if (elife->dev.cmd){
		json_object_object_add(params, "cmd", json_object_new_string(elife->dev.cmd));
	}
	if(elife ->dev.position){
		json_object_object_add(params, "position", json_object_new_string(elife->dev.position));
	}
	return _send_obj(elife,params);
}

//---------------------------------------------------- send_scene
char *send_scene(elife_t *elife){
	DEBUG("++++++++++++++++++++++ send_scene \n");
	json_object *params = NULL;

	params = json_object_new_object();
	if (params == NULL){
		DEBUG("-------------------------- params error\n");
		return NULL;
	}
	if(elife->scene.name){
		json_object_object_add(params, "name", json_object_new_string(elife->scene.name));
	}
	if (elife->scene.cmd){
		json_object_object_add(params, "cmd", json_object_new_string(elife->scene.cmd));
	}
	return _send_obj(elife,params);
}

//---------------------------------------------------- send_scene
char *send_security(elife_t *elife){
	DEBUG("++++++++++++++++++++++ send_security \n");
	json_object *params = NULL;

	params = json_object_new_object();
	if (params == NULL){
		DEBUG("-------------------------- params error\n");
		return NULL;
	}
	if(elife->security.name){
		json_object_object_add(params, "name", json_object_new_string(elife->security.name));
	}
	if (elife->security.cmd){
		json_object_object_add(params, "cmd", json_object_new_string(elife->security.cmd));
	}
	return _send_obj(elife,params);
}



//---------------------------------------------------- voice
char *send_voice(elife_t *elife){
	DEBUG("++++++++++++++++++++++ send_voice \n");
	json_object *params = NULL;

	params = json_object_new_object();
	if (params == NULL){
		DEBUG("-------------------------- params error\n");
		return NULL;
	}

	if(elife->video.name){
		json_object_object_add(params, "name", json_object_new_string(elife->video.name));
	}
	if (elife->video.cmd){
		json_object_object_add(params, "cmd", json_object_new_string(elife->video.cmd));
	}
	if(elife ->video.director){
		json_object_object_add(params, "director", json_object_new_string(elife->video.director));
	}
	if(elife ->video.actor){
		json_object_object_add(params, "actor", json_object_new_string(elife->video.actor));
	}
	if(elife ->video.v_type){
		json_object_object_add(params, "v_type", json_object_new_string(elife->video.v_type));
	}
	if(elife ->video.area){
		json_object_object_add(params, "area", json_object_new_string(elife->video.area));
	}
	if(elife ->video.episode_no){
		json_object_object_add(params, "episode_no", json_object_new_string(elife->video.episode_no));
	}
	return _send_obj(elife,params);
}

