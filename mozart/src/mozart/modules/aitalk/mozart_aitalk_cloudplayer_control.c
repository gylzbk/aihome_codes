#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <json-c/json.h>

//#include "dl_perform.h"
#include "utils_interface.h"
#include "player_interface.h"
#include "volume_interface.h"
#include "linklist_interface.h"
#include "tips_interface.h"
#include "event_interface.h"
#include "localplayer_interface.h"

#include "mozart_module.h"
#include "mozart_misc.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_net.h"
#include "mozart_player.h"

#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"

#include "vr-speech_interface.h"

#include "mozart_config.h"
#include "vr-speech_interface.h"
#include "aiengine_app.h"

#include "mozart_key.h"

#ifndef MOZART_RELEASE
#define MOZART_AITALK_CLOUDPLAYER_CONTROL_DEBUG
#endif

#ifdef MOZART_AITALK_CLOUDPLAYER_CONTROL_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[AITALK_CLOUDPLAYER_CONTROL] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_CLOUDPLAYER_CONTROL_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_CLOUDPLAYER_CONTROL_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[AITALK_CLOUDPLAYER_CONTROL] [Error] %s: "fmt, __func__, ##args)
#define pr_info(fmt, args...)			\
	printf("[AITALK_CLOUDPLAYER_CONTROL] [Info] %s: "fmt, __func__, ##args)

#define APP_PATH "/var/run/doug/hub_app.sock"
#define HOST_PATH "/var/run/doug/hub_host.sock"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct aitalk_method {
	const char *name;
	int (*handler)(json_object *cmd);
	bool (*is_valid)(json_object *cmd);
};

struct aitalk_struct {
	/* socket */
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int server_sockfd;
	int client_sockfd;

	struct dl_perform *dp;
	pthread_t down_thread;
	List up_queue_list;
} aitalk = {
	.server_sockfd = -1,
	.client_sockfd = -1,
};

static char *play_prompt = NULL;
static char *current_url = NULL;
bool aitalk_is_playing = false;

static bool is_aitalk_run = false;
bool is_aitalk_asr = false;

static bool aitalk_running = false;
static bool aitalk_initialized = false;
static player_handler_t *aitalk_player_handler;

enum aitalk_wait_stop_enum {
	aitalk_wait_stop_invalid,
	aitalk_wait_stop_waitstop,
	aitalk_wait_stop_stopped,
} aitalk_wait_stop_state;

static pthread_mutex_t aitalk_wait_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t aitalk_wait_stop_cond = PTHREAD_COND_INITIALIZER;




struct update_msg {
	char *uri;
	char *title;
	char *artist;
};
//static pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
//static int up_die;
//static pthread_cond_t	cond = PTHREAD_COND_INITIALIZER;
//static pthread_mutex_t	cond_lock = PTHREAD_MUTEX_INITIALIZER;
#if 1
#define send_result_obj(cmd, result_obj)	\
	send_obj(cmd, NULL, result_obj)
#define send_notification_obj(method, method_obj)	\
	send_obj(NULL, method, method_obj)

static int send_obj(json_object *cmd, char *method, json_object *obj)
{
	return 0;
	const char *s;
	json_object *o, *m, *id = NULL;

	o = json_object_new_object();
	json_object_object_add(o, "jsonrpc", json_object_new_string("2.0"));
	if (cmd && json_object_object_get_ex(cmd, "id", &id)) {
		/* result */
		json_object_object_get_ex(cmd, "method", &m);
		json_object_object_add(o, "method", m);
		json_object_object_add(o, "result", obj);
		json_object_object_add(o, "id", id);

	} else if (method) {
		/* notification */
		json_object_object_add(o, "method", json_object_new_string(method));
		json_object_object_add(o, "params", obj);
	} else {
		json_object_put(o);
		printf("%s: send fail!\n", __func__);
		return -1;
	}

	s = json_object_to_json_string(o);
	pr_debug("<<<< %s: %s: %s\n", __func__, id ? "Result" : "Notifcation", s);
//	hub_send((char *)s, strlen(s));

	json_object_put(o);

	return 0;
}

typedef enum player_state {
	player_play_state = 0,
	player_pause_state,
	player_stop_state,
}player_state;

player_state ai_play_state;

static int send_player_state_change(enum player_state state)
{

	#if 0
	json_object *params;

	params = json_object_new_object();
	json_object_object_add(params, "state", json_object_new_int(state));

	if (send_notification_obj("player_state_change", params)) {
		json_object_put(params);
		return -1;
	}
	#else
		ai_play_state = state;
	#endif

	return 0;
}

static int send_play_done(const char *url, int error_no)
{
	#if 0
	json_object *params, *error = NULL;

	printf("%s...!\n",__func__);
	params = json_object_new_object();
	json_object_object_add(params, "uri", json_object_new_string(url));
	if (error_no) {
		error = json_object_new_object();
		json_object_object_add(error, "code", json_object_new_int(error_no));

		json_object_object_add(params, "status", json_object_new_int(1));
		json_object_object_add(params, "error", error);
	} else {
		json_object_object_add(params, "status", json_object_new_int(0));
	}

	if (send_notification_obj("play_done", params)) {
		if (error)
			json_object_put(error);
		json_object_put(params);
		return -1;
	}
	#else
	//----------- conture playing
	if (ai_is_play_music()== true){
		ai_play_music_order(1);;
	}
	else{
		aitalk_is_playing = false;
	}
	#endif
	return 0;
}

static int send_player_volume_change(int volume)
{
	#if 0
	json_object *params;

	printf("%s...!\n",__func__);
	params = json_object_new_object();
	json_object_object_add(params, "volume", json_object_new_int(volume));

	if (send_notification_obj("player_volume_change", params)) {
		json_object_put(params);
		return -1;
	}
	#endif
	return 0;
}

static int send_button_event(char *name, char *str, char *value)
{
	#if 0
	json_object *params;

	printf("%s...!\n",__func__);
	params = json_object_new_object();
	json_object_object_add(params, "name", json_object_new_string(name));
	if (str && value)
		json_object_object_add(params, str, json_object_new_string(value));

	if (send_notification_obj("button", params)) {
		json_object_put(params);
		return -1;
	}
	#endif
	return 0;
}
#endif

int mozart_aitalk_start(void){
	ai_aiengine_start();
	return 0;
}

int mozart_aitalk_stop(void){
//	mozart_aitalk_asr_over();
	ai_aiengine_stop();
	mozart_key_ignore_set(false);
	return 0;
}


/*******************************************************************************
 * handler
 *******************************************************************************/
static bool vendor_is_valid(json_object *cmd)
{
	return true;//aitalk_cloudplayer_monitor_is_valid();
}

static int wakeup_handler(json_object *cmd)
{
	pr_debug("wakeup_handler...\n");
	mozart_module_asr_wakeup();
	return 0;
}

static int play_handler(json_object *cmd)
{
	int ret;
	const char *url;
	json_object *params, *url_j, *artist, *title, *vendor = NULL;

	if (!json_object_object_get_ex(cmd, "params", &params)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "url", &url_j)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "title", &title)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "artist", &artist)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	//if (!json_object_object_get_ex(params, "vendor", &vendor))
	//	return -1;

	url = json_object_get_string(url_j);
	printf("    url: %s\n", url);

	if ((url == NULL) || (url[0] == '/' &&
		access(url, R_OK))) {
		printf("[%s %s %d]: error\n", __FILE__, __func__, __LINE__);
		send_play_done(url, 0);
		return 0;
	}

#if SUPPORT_SMARTUI
	pr_debug("play_prompt = %s\n",play_prompt);
	char *title_s =(char *)json_object_get_string(title);
	char *artist_s =  (char *)json_object_get_string(artist);

	if((artist_s)||(artist_s)){
		mozart_smartui_atalk_play("AIspeech",title_s,artist_s,play_prompt);
	}
#endif

	ret = mozart_aitalk_cloudplayer_do_play();
	if (ret == 0) {
		pr_debug("cloudplayer module isn't run\n");
	} else if (ret < 0) {
		pr_debug("cloudplayer module isn't start\n");
		send_player_state_change(player_stop_state);
		return -1;
	}
	if(url == NULL){
		return -1;
	}

	free(current_url);
	current_url = NULL;
	current_url = strdup(url);

	if (ret > 0) {
		if (mozart_player_playurl(aitalk_player_handler, (char *)url))
			printf("[Warning] %s: mozart_player_playurl fail\n", __func__);
		send_player_state_change(player_play_state);
	} else {
		char *uuid = mozart_player_getuuid(aitalk_player_handler);
		mozart_aitalk_cloudplayer_update_context(uuid, (char *)url);
		send_player_state_change(player_pause_state);
		free(uuid);
	}

	aitalk_is_playing = true;
#if SUPPORT_SMARTUI
	mozart_smartui_atalk_play((char *)json_object_get_string(vendor),
				  (char *)json_object_get_string(title),
				  (char *)json_object_get_string(artist),
				  play_prompt);
#endif
	return 0;
}

static int stop_handler(json_object *cmd)
{
	mozart_aitalk_cloudplayer_do_stop();

	return 0;
}

static int pause_handler(json_object *cmd)
{
	mozart_aitalk_cloudplayer_do_pause();

	return 0;
}

static int resume_handler(json_object *cmd)
{
	mozart_aitalk_cloudplayer_do_resume();

	return 0;
}

static int pause_toggle_handler(json_object *cmd)
{
	mozart_aitalk_cloudplayer_do_resume_pause();

	return 0;
}

static int set_volume_handler(json_object *cmd)
{
	const char *volume;
	int vol = 0;
	printf("%s...!\n",__func__);
	json_object *params, *volume_j;//, *artist, *title, *vendor;

	if (!json_object_object_get_ex(cmd, "params", &params)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}
	if (!json_object_object_get_ex(params, "volume", &volume_j)){
		pr_err("[%d]error!\n",__LINE__);
		return -1;
	}

	volume = json_object_get_string(volume_j);
	if (volume){
		vol = mozart_volume_get();
		pr_debug("volume = %s, last vol = %d!\n",volume,vol);
		if(strcmp(volume, "+") == 0){
			vol +=10;
		}else if(strcmp(volume, "-") == 0){
			vol -= 10;
		}else{
			vol = atoi(volume);
		}	//*/
		if (vol > 100)
			vol = 100;
		else if (vol < 0)
			vol = 0;

		pr_debug("new vol = %d!\n",vol);
		mozart_volume_set(vol, MUSIC_VOLUME);
	}
	else{
		return -1;
	}
/*XXX*/
#if 0
	if(aitalk_play_music == true){
		mozart_aitalk_cloudplayer_do_resume();
	}
#endif
	return 0;
}

static int play_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	ai_play_music_order(0);
	return 0;
}


static int previous_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	ai_play_music_order(-1);
	return 0;
}

static int next_music_handler(json_object *cmd)
{
	printf("%s...!\n",__func__);
	ai_play_music_order(1);
	return 0;
}


static int play_voice_prompt_handler(json_object *cmd)
{
	char mp3_src[256] = {0};
	const char *url;
	json_object *params, *uri;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;
	if (!json_object_object_get_ex(params, "uri", &uri))
		return -1;

	url = json_object_get_string(uri);

	if (url == NULL){
		return -1;
	}


	if (!play_tone_get_source(mp3_src, "atalk_entry_13") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("栏目订阅");
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_14") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("音乐电台");
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_15") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("随便听听");
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_16") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("广播电台");
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_17") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("音乐收藏");
	} else if (!play_tone_get_source(mp3_src, "atalk_local_4") && !strcmp(url, mp3_src)) {
		pr_debug("skip local_4\n");
		return 0;
	} else if (!play_tone_get_source(mp3_src, "atalk_hi_12") && !strcmp(url, mp3_src)) {
		bool module_change;

		mozart_module_mutex_lock();
		module_change = __mozart_aitalk_cloudplayer_is_start();
		if (!__mozart_net_is_start() && !__mozart_aitalk_localplayer_is_start()){
		 // &&   !atalk_cloudplayer_monitor_is_module_cancel())
			if (mozart_aitalk_cloudplayer_start(true)) {
				pr_err("start fail!\n");
				mozart_module_mutex_unlock();
				return -1;
			} else {
				if (!module_change)
					send_play_done("NULL", 0);
				mozart_smartui_boot_welcome();
			}
			//atalk_cloudplayer_monitor_cancel();
		}
		mozart_module_mutex_unlock();
	}

	mozart_module_mutex_lock();

	if (!__mozart_aitalk_cloudplayer_is_run() || !vendor_is_valid(NULL)) {
		pr_debug("[Warning] %s: Don't play %s\n", __func__, url);
	} else {
		if (!strncmp(url, "file://", 7))
			mozart_prompt_tone_play_sync((char *)url + 7, true);
		else
			mozart_prompt_tone_play_sync((char *)url, true);
	}
	mozart_module_mutex_unlock();

	return 0;
}

static int net_state_change_handler(json_object *cmd)
{
	json_object *params, *state;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;

	if (!json_object_object_get_ex(params, "state", &state))
		return -1;

/*	if (json_object_get_int(state) == 0)
		mozart_aitalk_net_change(false);
	else
		mozart_aitalk_net_change(true);
//*/
	return 0;
}

#if 0
static int update_cache_handler(json_object *cmd)
{
	struct update_msg *msg;
	json_object *params, *uri_j, *title_j, *artist_j, *vendor_j;
	const char *uri = NULL;
	const char *title = NULL;
	const char *artist = NULL;

/*	if (up_die) {
		printf("[Error] %s. Download function not work\n", __func__);
		return -1;
	}		//*/

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;
	if (!json_object_object_get_ex(params, "uri", &uri_j))
		return -1;
	if (!json_object_object_get_ex(params, "title", &title_j))
		return -1;
	if (!json_object_object_get_ex(params, "artist", &artist_j))
		return -1;
	if (!json_object_object_get_ex(params, "vendor", &vendor_j))
		return -1;

	msg = malloc(sizeof(struct update_msg));
	if (!msg) {
		printf("[Error] %s. Alloc message: %s\n", __func__, strerror(errno));
		return -1;
	}

	uri = json_object_get_string(uri_j);
	title = json_object_get_string(title_j);
	artist = json_object_get_string(artist_j);

	if (!uri || !title || !artist) {
		printf("[Error] %s. uri or title: %s\n", __func__, strerror(errno));
		goto err_node_str;
	}

	if(uri){
		msg->uri = strdup(uri);
	}
	if(title){
		msg->title = strdup(title);
	}
	if(title){
		msg->artist = strdup(artist);
	}

/*	pthread_mutex_lock(&q_lock);
	if (up_die) {
		pthread_mutex_unlock(&q_lock);
		goto out_up_die;
	}		//*/
	/* Insert at queue tail */
//	list_insert_at_tail(&atalk.up_queue_list, msg);
//	pthread_mutex_unlock(&q_lock);

//	pthread_mutex_lock(&cond_lock);
//	pthread_cond_signal(&cond);
//	pthread_mutex_unlock(&cond_lock);

	return 0;

err_node_str:
out_up_die:
	if (msg->uri)
		free(msg->uri);
	if (msg->title)
		free(msg->title);
	if (msg->artist)
		free(msg->artist);

	free(msg);

	return -1;
}
#endif

static int is_attaching_handler(json_object *cmd)
{
	json_object *result = json_object_new_object();

	mozart_module_mutex_lock();
	if (__mozart_module_is_attach())
		json_object_object_add(result, "attach", json_object_new_string("true"));
	else
		json_object_object_add(result, "attach", json_object_new_string("false"));
	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		mozart_module_mutex_unlock();
		return -1;
	} else {
		mozart_module_mutex_unlock();
		return 0;
	}
}

static int get_mac_address_handler(json_object *cmd)
{
	char macaddr[] = "000.000.000.000";
	json_object *result = json_object_new_object();

	get_mac_addr("wlan0", macaddr, NULL);
	json_object_object_add(result, "mac_address",
			       json_object_new_string(macaddr));

	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		return -1;
	} else {
		return 0;
	}
}

static int get_setup_wifi_handler(json_object *cmd)
{
	json_object *result = json_object_new_object();

	mozart_module_mutex_lock();
	if (__mozart_module_is_online())
		json_object_object_add(result, "state", json_object_new_int(1));
	else if (__mozart_module_is_net())
		json_object_object_add(result, "state", json_object_new_int(0));
	else
		json_object_object_add(result, "state", json_object_new_int(0));
	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		mozart_module_mutex_unlock();
		return -1;
	} else {
		mozart_module_mutex_unlock();
		return 0;
	}
}

static int get_volume_handler(json_object *cmd)
{
	char volume[8];
	json_object *result = json_object_new_object();

	sprintf(volume, "%d", mozart_volume_get());
	json_object_object_add(result, "volume", json_object_new_string(volume));

	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		return -1;
	} else {
		return 0;
	}
}

static bool module_is_attach(json_object *cmd)
{
	bool is_attach;
	return true;
//-------------------------------- not checkout:
	mozart_module_mutex_lock();
	is_attach = __mozart_module_is_attach();
	mozart_module_mutex_unlock();

	return is_attach;
}

static struct aitalk_method methods[] = {
	{
		.name = "wakeup",
		.handler = wakeup_handler,
		.is_valid = vendor_is_valid,
	},
	{
		.name = "play",
		.handler = play_handler,
		.is_valid = vendor_is_valid,
	},
	{
		.name = "stop",
		.handler = stop_handler,
	},
	{
		.name = "pause",
		.handler = pause_handler,
	},
	{
		.name = "resume",
		.handler = resume_handler,
	},
	{
		.name = "pause_toggle",
		.handler = pause_toggle_handler,
	},
	{
		.name = "set_volume",
		.handler = set_volume_handler,
		.is_valid = module_is_attach,
	},
	{
		.name = "play_music",
		.handler = play_music_handler,
	},
	{
		.name = "next_music",
		.handler = next_music_handler,
	},
	{
		.name = "previous_music",
		.handler = previous_music_handler,
	},
	{
		.name = "play_voice_prompt",
		.handler = play_voice_prompt_handler,
	},
	{
		.name = "net_state_change",
		.handler = net_state_change_handler,
	},

	/* result */
	{
		.name = "is_attaching",
		.handler = is_attaching_handler,
	},
	{
		.name = "get_mac_address",
		.handler = get_mac_address_handler,
	},
	{
		.name = "get_setup_wifi_state",
		.handler = get_setup_wifi_handler,
	},
	{
		.name = "get_volume",
		.handler = get_volume_handler,
	},
};

/*******************************************************************************
 * Player
 *******************************************************************************/
static int aitalk_player_status_callback(player_snapshot_t *snapshot,
					struct player_handler *handler, void *param)
{
	if (strcmp(handler->uuid, snapshot->uuid))
		return 0;

	pr_debug("status = %d, %s\n", snapshot->status, player_status_str[snapshot->status]);

	if (snapshot->status == PLAYER_STOPPED) {
		/* for __stop_handler */
		pthread_mutex_lock(&aitalk_wait_stop_mutex);
		if (aitalk_wait_stop_state == aitalk_wait_stop_waitstop) {
			pr_debug("aitalk_wait_stop_state is WAIT_STOP\n");
			aitalk_wait_stop_state = aitalk_wait_stop_stopped;
		} else {

			send_play_done(current_url, 0);
		}
		pthread_mutex_unlock(&aitalk_wait_stop_mutex);
		aitalk_is_playing = false;
	}

	return 0;
}

int aitalk_cloudplayer_resume_player(void)
{
	int ret;

	send_player_state_change(player_play_state);
	ret = mozart_player_resume(aitalk_player_handler);
	if (ret)
		printf("[Warning] %s: mozart_player_resume fail\n", __func__);

	return ret;
}

int aitalk_cloudplayer_pause_player(void)
{
	int ret;

	send_player_state_change(player_pause_state);
	ret = mozart_player_pause(aitalk_player_handler);
	if (ret)
		printf("[Warning] %s: mozart_player_pause fail\n", __func__);

	ret = mozart_player_wait_status(aitalk_player_handler, PLAYER_PAUSED, 500 * 1000);

	return ret;
}

int aitalk_cloudplayer_stop_player(void)
{
	long usec;
	struct timeval now;
	struct timespec timeout;

	send_player_state_change(player_stop_state);

	if (!aitalk_is_playing)
		return 0;

	pthread_mutex_lock(&aitalk_wait_stop_mutex);

	aitalk_wait_stop_state = aitalk_wait_stop_waitstop;

	if (mozart_player_stop(aitalk_player_handler))
		printf("[Warning] %s: mozart_player_stop fail\n", __func__);

	gettimeofday(&now, NULL);
	usec = now.tv_usec + 500 * 1000;
	timeout.tv_sec = now.tv_sec + usec / 1000000;
	timeout.tv_nsec = (usec % 1000000) * 1000;

	pthread_cond_timedwait(&aitalk_wait_stop_cond, &aitalk_wait_stop_mutex, &timeout);
	if (aitalk_wait_stop_state != aitalk_wait_stop_stopped) {
		pr_err("wait stopped timeout\n");
		mozart_player_force_stop(aitalk_player_handler);
		aitalk_is_playing = false;
	}

	aitalk_wait_stop_state = aitalk_wait_stop_invalid;

	pthread_mutex_unlock(&aitalk_wait_stop_mutex);

	return 0;
}

#if 0
/*******************************************************************************
 * Download
 *******************************************************************************/
void end_func(DPres_t res, char *errStr, void *userData)
{
	int *result = (int *)userData;

	switch (res) {
	case DP_OK:
		*result = 0;
		break;
	default:
		printf("[ERROR].aitalk download: %s\n", errStr);
		*result = -1;
	}
}

#define TIMEOUT 4

static int aitalk_update_download(const char *uri, const char *save)
{
	char *tempfile = NULL;
	int res;
	int err = 0;

	if (!strstr(uri, "://")) {
		printf("[Error] %s. '%s' is not URI scheme\n", __func__, uri);
		return -1;
	}

	tempfile = malloc(strlen(save) + sizeof(".ucache"));
	if (!tempfile) {
		printf("[Error] %s. Alloc tempfile: %s\n", __func__, strerror(errno));
		return -1;
	}

	snprintf(tempfile, strlen(save) + sizeof(".ucache"), "%s.ucache", save);

	aitalk.dp->endFunc	= end_func;
	aitalk.dp->endData	= &res;
	aitalk.dp->maxSpeedLimit = 64 * 1024; /* Limit download speed 64KB */

	err = dl_perform_sync(aitalk.dp, (char *)uri, tempfile, 0);
	if (err < 0)
		goto err_dl;

	if (!res) {
		err = rename(tempfile, save);
		if (err < 0) {
			printf("[Error]%s. rename: %s\n", __func__, strerror(errno));
			unlink(tempfile);
		}
	} else {
		unlink(tempfile);
		err = -1;
	}

err_dl:
	free(tempfile);

	return err;
}
#undef TIMEOUT

static int aitalk_update_cache_clear(const char *dir, char *key)
{
	DIR *dir_p;
	struct dirent *entry;
	int count = 0;

	dir_p = opendir(dir);
	if (!dir_p) {
		printf("[Error] %s. opendir: %s\n", __func__, strerror(errno));
		return -1;
	}

	while ((entry = readdir(dir_p)) != NULL) {
		/* Unlink cache file */
		if (strstr(entry->d_name, key)) {
			char *target = malloc(strlen(dir) + strlen(entry->d_name) + sizeof("/"));
			if (!target) {
				printf("[Error] %s. Alloc unlink target: %s\n", __func__, strerror(errno));
				closedir(dir_p);
				return -1;
			}

			sprintf(target, "%s/%s", dir, entry->d_name);
			unlink(target);
			pr_debug("[Debug] clear file: %s\n", entry->d_name);

			free(target);
			count++;
		}
	}

	closedir(dir_p);

	printf("[Info] %s. clear %d favorite caches\n", __func__, count);

	return 0;
}

static int aitalk_queue_head(const void *data, const void *key)
{
	return 0;
}


static void aitalk_list_destroy(void *data)
{
	struct update_msg *msg = (struct update_msg *)data;
	if (msg->uri)
		free(msg->uri);
	if (msg->title)
		free(msg->title);
	if (msg->artist)
		free(msg->artist);
	free(msg);
}
static void *aitalk_update_queue_handle_func(void *data)
{
	pthread_detach(pthread_self());
	char dir_target[] = {"/mnt/sdcard/aitalk-favorite"};
	char *path_title = NULL;
	int title_size;
	struct stat st;
	struct update_msg *msg;
	int scan_flag = 0;
	int err;

	if (!mozart_path_is_mount("/mnt/sdcard"))
		goto err_pre;

	/* Check download directory */
	err = stat(dir_target, &st);
	if (err < 0) {
		if (errno == ENOENT) {
			mkdir(dir_target, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		} else {
			printf("[Error] %s. stat: %s\n", __func__, strerror(errno));
			goto err_pre;
		}
	} else {
		aitalk_update_cache_clear(dir_target, ".ucache");
	}

	while (!up_die) {
		while (!up_die && !is_empty(&aitalk.up_queue_list)) {
			pthread_mutex_lock(&q_lock);
			/* Get queue at head */
			msg = list_delete(
				&aitalk.up_queue_list,
				NULL,
				aitalk_queue_head);
			pthread_mutex_unlock(&q_lock);

			if (strstr(msg->uri, "file://")) {
				printf("[Info] %s. %s is local file\n", __func__, msg->uri);
				goto up_file_pass;
			}

			if (!mozart_ext_storage_enough_free("/mnt/sdcard", 128 * 1024 * 1024)) {
				pr_info("Has not enough space, clear aitalk-favorite\n");
				mozart_ext_storage_file_clear("/mnt/sdcard/aitalk-favorite");
			}

			title_size = strlen(dir_target) +
				     strlen(msg->artist) +
				     strlen(msg->title) +
				     sizeof("/+++.mp3");
			path_title = malloc(title_size);
			if (!path_title) {
				printf("%s. Alloc path_title: %s", __func__, strerror(errno));
				up_die = 1;
				goto up_file_pass;
			}
			memset(path_title, 0, title_size);

			/* Check file exist */
			snprintf(path_title, title_size,
				"%s/%s+++%s.mp3",
				dir_target,
				msg->artist,
				msg->title);
			err = stat(path_title, &st);
			if (!err) {
				printf("[Info] %s. %s exist\n", __func__, path_title);
				goto up_file_pass;
			}

			if (errno == ENOENT) {
				err = aitalk_update_download(msg->uri, path_title);
				if (!err) {
					/* Fresh play list */
					scan_flag = 1;
				}
			} else {
				printf("[Error] %s. stat: %s\n", __func__, strerror(errno));
			}

up_file_pass:
			if (msg->uri)
				free(msg->uri);
			if (msg->title)
				free(msg->title);
			if (msg->artist)
				free(msg->artist);
			free(msg);

			if (path_title) {
				free(path_title);
				path_title = NULL;
			}
		}

		if (!up_die && scan_flag) {
			mozart_localplayer_scan();
			scan_flag = 0;
			continue;
		}

		pthread_mutex_lock(&cond_lock);
		if (!up_die)
			pthread_cond_wait(&cond, &cond_lock);
		pthread_mutex_unlock(&cond_lock);
	}

	pthread_mutex_lock(&q_lock);
	list_destroy(&aitalk.up_queue_list, aitalk_list_destroy);
	pthread_mutex_unlock(&q_lock);

err_pre:
	up_die = 1;

	return NULL;
}
#endif

/*******************************************************************************
 * API
 *******************************************************************************/
int aitalk_cloudplayer_send_wifi_state(enum wifi_state state)
{
	json_object *params;

	params = json_object_new_object();
	json_object_object_add(params, "state", json_object_new_int(state));

	if (send_notification_obj("setup_wifi_state_change", params)) {
		json_object_put(params);
		return -1;
	}

	return 0;
}

int __aitalk_switch_mode(bool attach)
{
	int ret = 0;

	if (attach) {
		if (!__mozart_module_is_attach()) {
			__mozart_module_set_attach();
			ret = send_button_event("switch_mode", "attach", "true");
		}
	} else {
		if (__mozart_module_is_attach()) {
			__mozart_module_set_unattach();
			ret = send_button_event("switch_mode", "attach", "false");
		}
	}

	return ret;
}

int aitalk_cloudplayer_volume_change(int vol)
{
	send_player_volume_change(vol);
	return 0;
}

int aitalk_cloudplayer_volume_set(int vol)
{
	mozart_volume_set(vol, MUSIC_VOLUME);
	send_player_volume_change(vol);

	return 0;
}

int aitalk_cloudplayer_previous_music(void)
{
	return send_button_event("previous", NULL, NULL);
}

int aitalk_cloudplayer_next_music(void)
{
	return send_button_event("next", NULL, NULL);
}

int aitalk_next_channel(void)
{
	return send_button_event("next_channel", NULL, NULL);
}

int aitalk_love_audio(void)
{
	return send_button_event("love_audio", "uri", current_url);
}

void aitalk_vendor_startup(void)
{
	mozart_aitalk_start();
}

void aitalk_vendor_shutdown(void)
{
//	mozart_system("killall aitalk_vendor");
//	unlink("/var/run/doug.pid");
	mozart_aitalk_stop();
	mozart_key_ignore_set(false);
}

static void *aitalk_running_func(void *args)
{
	pthread_detach(pthread_self());
	char *cmd = NULL;//[512]={}
	const char *method = NULL;
	json_object *c = NULL;
	json_object	*o = NULL;
	bool is_valid = true;
	struct aitalk_method *m;
	is_aitalk_run = true;
	aitalk_running= true;
	while (aitalk_running) {
		int i;
		if (ai_aitalk_handler_wait() == -1){
			pr_err("ai_aitalk_handler_wait error!\n");
		}
		if (aitalk_running == false){
			//----------- extern running;
			break;
		}
		cmd = ai_aitalk_receive();
		if (cmd){
		//	pr_debug(">>>> %s: Recv: %s\n", __func__, cmd);
			c = json_tokener_parse(cmd);
			if (c){
				if (json_object_object_get_ex(c, "method", &o)){
					method = json_object_get_string(o);
					for (i = 0; i < ARRAY_SIZE(methods); i++) {
						m = &methods[i];
						if (m->name){
							if (!strcmp(m->name, method)) {
								if (m->is_valid)
									is_valid = m->is_valid(c);	//*/
								if (is_valid){
									pr_debug("method = %s\n", m->name);
									m->handler(c);
								}
								else
									pr_debug("     %s invalid\n", cmd);
								break;
							}
						}
					}

					if (i >= ARRAY_SIZE(methods))
						pr_debug("invalid command: %s\n", method);

					json_object_put(c);
				}
			}
			else{
				pr_debug("[%s:%d] json_tokener_parse c error: %s\n", __func__,__LINE__, cmd);
			}
		}
		usleep(1000);
	}
	is_aitalk_run= false;
	return NULL;
}

int mozart_vr_speech_interface_callback(vr_info *recog)
{
	int ret = 0;
	switch(recog->status){
		case AIENGINE_STATUS_INIT:
		case AIENGINE_STATUS_AEC: {
			is_aitalk_asr = false;
			mozart_key_ignore_set(false);
			#if SUPPORT_SMARTUI
				mozart_smartui_asr_over();
				if(recog->domain == RECOG_DOMAIN_WEATHER) {
#if SUPPORT_BOARD==BOARD_WB38
					mozart_smartui_weather_start(recog->weather);
#endif
				}
			#endif
	    	}
			break;
		case AIENGINE_STATUS_SEM: {
			is_aitalk_asr = true;
			#if SUPPORT_SMARTUI
				mozart_smartui_asr_start();
			#endif
			mozart_prompt_tone_key_sync("welcome", false);
		}
		break;

		case AIENGINE_STATUS_PROCESS: {
			#if SUPPORT_SMARTUI
				mozart_smartui_asr_recognize();
			#endif
		}
		break;

		default:
		break;
	}

//exit_error:
	if (ret == 0){
		if (__mozart_aitalk_network_state() == network_online){
			ret = 0;
		}
		else{
			ret = 1;
		}
	}
	return ret;
}


int aitalk_cloudplayer_startup(void)
{
	pthread_t aitalk_thread;

	if (!aitalk_initialized) {
		aitalk_player_handler =
			mozart_player_handler_get("aitalk", aitalk_player_status_callback, NULL);
		if (aitalk_player_handler == NULL) {
			printf("%s: get_player_handler fail!\n", __func__);
			return -1;
		}

		if (pthread_create(&aitalk_thread, NULL, aitalk_running_func, NULL) != 0) {
			printf("%s: Can't create aitalk_thread: %s\n",
			       __func__, strerror(errno));
			return -1;
		}
		aitalk_initialized = true;
	}

	return 0;
}


int aitalk_cloudplayer_shutdown(void)
{
	pthread_mutex_lock(&aitalk_wait_stop_mutex);
	pthread_cond_signal(&aitalk_wait_stop_cond);
	pthread_mutex_unlock(&aitalk_wait_stop_mutex);

	free(play_prompt);
	play_prompt = NULL;
	free(current_url);
	current_url = NULL;

//------------------------- wake stop
	aitalk_running = false;
	int count = 0;
	while(is_aitalk_run){
		usleep(1000);
		count ++;
		if (count >   1000){
			break;
		}
	}

	mozart_module_mutex_lock();
	__mozart_module_set_attach();
	__mozart_module_set_offline();
	__mozart_module_set_net();
	mozart_module_mutex_unlock();

	mozart_aitalk_cloudplayer_do_stop();


//	up_die = 1;
//	dl_perform_stop(aitalk.dp);

//	pthread_mutex_lock(&cond_lock);
//	pthread_cond_signal(&cond);
//	pthread_mutex_unlock(&cond_lock);

//	pthread_join(aitalk.down_thread, NULL);
//	dl_perform_uninit(aitalk.dp);

	//aitalk_vendor_shutdown();

	aitalk_initialized = false;
	aitalk_vendor_shutdown();

	if (aitalk_player_handler) {
		mozart_player_handler_put(aitalk_player_handler);
		aitalk_player_handler = NULL;
	}

	return 0;
}




