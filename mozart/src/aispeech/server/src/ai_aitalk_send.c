#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <semaphore.h>
#include <json-c/json.h>
#include "vr-speech_interface.h"
#include "aiengine.h"

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
static sem_t sem_aitalk;
char *aitalk_pipe_buf = NULL;
static bool is_aitalk_send_init = false;

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


char *aitalk_send_waikup(const char *url){
	return send_obj("wakeup",NULL);
}

int ai_aitalk_send_init(void){
    sem_init(&sem_aitalk, 0, 0);
	is_aitalk_send_init = true;
	return 0;

}

int ai_aitalk_send(char *data){
	if (!data){
		return -1;
	}
	if(is_aitalk_send_init == false){
		PERROR("is_aitalk_send_init = false \n");
		return -1;
	}
	DEBUG("ai_aitalk_send ==> %s\n",data);
	free(aitalk_pipe_buf);
	aitalk_pipe_buf = NULL;
	aitalk_pipe_buf = strdup(data);

	sem_post(&sem_aitalk);
	return 0;
}

char *ai_aitalk_receive(void){
	return aitalk_pipe_buf;
}


int ai_aitalk_send_destroy(void){
	ai_aitalk_send("exit");	//	exit send
	usleep(10000);
	free(aitalk_pipe_buf);
	aitalk_pipe_buf = NULL;
	free(pjson);
	pjson =   NULL;
	if(is_aitalk_send_init == true){
		sem_destroy(&sem_aitalk);
		is_aitalk_send_init = false;
	}
	return 0;
}

int ai_aitalk_handler_wait(void){
	if(is_aitalk_send_init == false){
		PERROR("is_aitalk_send_init = false \n");
		sleep(1);
		return -1;
	}

	sem_wait(&sem_aitalk);
	return 0;
}	//*/

