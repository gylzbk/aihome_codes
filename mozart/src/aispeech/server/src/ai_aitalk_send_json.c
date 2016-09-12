#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <json-c/json.h>
#include "vr-speech_interface.h"

#ifndef MOZART_RELEASE
#define MOZART_AITALK_DEBUG
#endif

#ifdef MOZART_AITALK_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[AITALK_JSON] %s: "fmt, __func__, ##args)
#else  /* MOZART_AITALK_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_AITALK_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[AITALK_JSON] [Error] %s: "fmt, __func__, ##args)


static char *pjson = NULL;

char *send_obj(char *method,json_object *obj)
{
	json_object *o;
	const char *s;
	free(pjson);
	pjson =   NULL;
	o = json_object_new_object();
	if (method) {
		/* notification */
		json_object_object_add(o, "method", json_object_new_string(method));
		json_object_object_add(o, "params", obj);
	} else {
		json_object_put(o);
		printf("%s: send fail!\n", __func__);
		goto exit_err;
	}

	s = json_object_to_json_string(o);
//	pr_debug("<<<< %s\n", s);
	if (s){
		pjson = strdup(s);
	}
	if (o){
		json_object_put(o);
	}
	else {
		if (obj){
			json_object_put(obj);
		}
	}
exit_err:
	return pjson;
}

char *aitalk_send_play_url(const music_info *music){

	char *artist = NULL;
	char *title = NULL;
	json_object *params;
	if (music->url == NULL){
		pr_err("Error : not url ...\n");
		return NULL;
	}

	artist = music->artist;
	title = music->title;
	if (title == NULL){
		title = "";
	}
	if (artist == NULL){
		artist = "";
	}
	params = json_object_new_object();
	json_object_object_add(params, "url", json_object_new_string(music->url));
	json_object_object_add(params, "title", json_object_new_string(title));
	json_object_object_add(params, "artist", json_object_new_string(artist));
//	pr_debug("%s params = %s\n", __func__,json_object_to_json_string(params));
	return send_obj("play",params);
}

char *aitalk_send_pause(const char *url){
	return send_obj("pause",NULL);
}

char *aitalk_send_resume(const char *url){
	return send_obj("resume",NULL);
}
char *aitalk_send_stop_music(const char *url){
	return send_obj("stop_music",NULL);
}

char *aitalk_send_play_music(const char *url){
	return send_obj("play_music",NULL);
}

char *aitalk_send_previous_music(const char *url){
	return send_obj("previous_music",NULL);
}

char *aitalk_send_next_music(const char *url){
	return send_obj("next_music",NULL);
}

char *aitalk_send_set_volume(const char *cmd){
	json_object *params;
	params = json_object_new_object();
	json_object_object_add(params, "volume", json_object_new_string(cmd));
//	pr_debug("%s params = %s\n", __func__,json_object_to_json_string(params));
	return send_obj("set_volume",params);
}



