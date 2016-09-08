#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <json-c/json.h>

#include "dl_perform.h"
#include "atalk_control.h"
#include "wifi_interface.h"
#include "tips_interface.h"
#include "utils_interface.h"
#include "player_interface.h"
#include "volume_interface.h"
#include "event_interface.h"
#include "sharememory_interface.h"
#include "linklist_interface.h"
#include "localplayer_interface.h"
#include "mozart_smartui.h"

#define APP_PATH "/var/run/doug/hub_app.sock"
#define HOST_PATH "/var/run/doug/hub_host.sock"

#define ATALK_CTL_DEBUG
#ifdef ATALK_CTL_DEBUG
#define pr_debug(fmt, args...)			\
	printf(fmt, ##args)

#define atalk_dump()							\
	printf("#### {%s, %s, %d} state: %s_%s_%s, play_state: %s ####\n", \
	       __FILE__, __func__, __LINE__,				\
	       __atalk_is_net() ? "AIRKISS" : "NORMAL",			\
	       __atalk_is_attaching() ? "ATTACH" : "UNATTACH",		\
	       __atalk_is_online() ? "ONLINE" : "OFFLINE",		\
	       atalk_play_state_str[atalk.play_state])
#else
#define pr_debug(fmt, args...)			\
	do {} while (0)
#define atalk_dump()				\
	do {} while (0)
#endif
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct atalk_method {
	const char *name;
	int (*handler)(json_object *cmd);
	bool (*is_valid)(json_object *cmd);
};

static bool atalk_initialized;
static int up_die = 0;
static player_handler_t *atalk_player_handler;
static pthread_mutex_t atalk_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *play_prompt;
static char *current_url;

static char *atalk_play_state_str[] = {
	"ATALK_PLAY_IDLE",
	"ATALK_PLAY_RUN",
	"ATALK_PLAY_PLAY",
	"ATALK_PLAY_PAUSE",
};

enum atalk_state_bit {
	ATALK_STATE_NET_BIT = 0,
	ATALK_STATE_ATTACH_BIT,
	ATALK_STATE_ONLINE_BIT,
};

/* effectivity if state is ATALK_STATE_ATTACH_ONLINE */
enum atalk_play_state {
	ATALK_PLAY_IDLE = 0,
	ATALK_PLAY_RUN,
	ATALK_PLAY_PLAY,
	ATALK_PLAY_PAUSE,
};

/* sharememroy:
 * shutdown: atalk app stop
 * running: switch mode false(UNATTACH)
 * pause: stop or pause (ATTACH)
 * playing: play (ATTACH)
 */

struct atalk_struct {
	/* socket */
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int server_sockfd;
	int client_sockfd;
	/* state */
	enum atalk_play_state play_state;
	unsigned int state_map;

	struct dl_perform *dp;
	pthread_t down_thread;
	List up_queue_list;
} atalk = {
	.server_sockfd = -1,
	.client_sockfd = -1,
	.play_state = ATALK_PLAY_IDLE,
	.state_map = 1 << ATALK_STATE_ATTACH_BIT | 1 << ATALK_STATE_ONLINE_BIT,
};

struct update_msg {
	char *uri;
	char *title;
	char *artist;
};
static pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;

/* pthread condition wait */
static pthread_cond_t	cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t	cond_lock = PTHREAD_MUTEX_INITIALIZER;

static void __pause_handler(void);
static void __stop_handler(void);
static int __switch_mode_handler(bool attach);

static inline bool __atalk_is_attaching(void)
{
	return atalk.state_map & (1 << ATALK_STATE_ATTACH_BIT);
}

static inline bool __atalk_is_online(void)
{
	return atalk.state_map & (1 << ATALK_STATE_ONLINE_BIT);
}

static inline bool __atalk_is_net(void)
{
	return atalk.state_map & (1 << ATALK_STATE_NET_BIT);
}

static inline void __atalk_set_attaching(void)
{
	atalk.state_map |= (1 << ATALK_STATE_ATTACH_BIT);
}

static inline void __atalk_clear_attaching(void)
{
	atalk.state_map &= ~(1 << ATALK_STATE_ATTACH_BIT);
}

static inline void __atalk_set_online(void)
{
	atalk.state_map |= (1 << ATALK_STATE_ONLINE_BIT);
}

static inline void __atalk_set_offline(void)
{
	atalk.state_map &= ~(1 << ATALK_STATE_ONLINE_BIT);
}

static inline void __atalk_set_net(void)
{
	atalk.state_map |= (1 << ATALK_STATE_NET_BIT);
}

static inline void __atalk_set_net_over(void)
{
	atalk.state_map &= ~(1 << ATALK_STATE_NET_BIT);
}

static inline bool __atalk_can_play(void)
{
	return __atalk_is_online() && __atalk_is_attaching();
}

static bool __atalk_app_is_stop(void)
{
	module_status status;

	if (share_mem_get(ATALK_DOMAIN, &status)) {
		printf("%s: share_mem_get fail!\n", __func__);
		return -1;
	}

	if (status == STATUS_SHUTDOWN)
		return 1;
	else
		return 0;
}

static int __atalk_prepare_play(void)
{
	mozart_event event = {
		.event = {
			.misc = {
				.name = "atalk",
				.type = "prepare_play",
			},
		},
		.type = EVENT_MISC,
	};

	if (!__atalk_can_play())
		return -1;

	mozart_event_send(event);

	/* running/shutdown -> pause */
	if (share_mem_set(ATALK_DOMAIN, STATUS_PAUSE))
		printf("%s: share_mem_set fail.\n", __func__);
	atalk.play_state = ATALK_PLAY_IDLE;

	return 0;
}

static void __atalk_stop_play(void)
{
	if (atalk.play_state != ATALK_PLAY_IDLE)
		__stop_handler();

	if (share_mem_set(ATALK_DOMAIN, STATUS_RUNNING))
		printf("%s: share_mem_set fail.\n", __func__);
}

/*******************************************************************************
 * hub send/recv
 *******************************************************************************/
static int hub_init(void)
{
	int sockfd;
	struct sockaddr_un *un;

	/* server */
	un = &atalk.server_addr;
	un->sun_family = AF_UNIX;
	strcpy(un->sun_path, HOST_PATH);
	unlink(HOST_PATH);
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("%s: socket fail: %s\n", __func__, strerror(errno));
		return -1;
	}
	if (bind(sockfd, (struct sockaddr *)un, sizeof(struct sockaddr_un))) {
		printf("%s: bind fail: %s\n", __func__, strerror(errno));
		close(sockfd);
		return -1;
	}
	atalk.server_sockfd = sockfd;

	/* client */
	un = &atalk.client_addr;
	un->sun_family = AF_UNIX;
	strcpy(un->sun_path, APP_PATH);
	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		printf("%s: socket fail: %s\n", __func__, strerror(errno));
		close(atalk.server_sockfd);
		atalk.server_sockfd = -1;
		close(sockfd);
		return -1;
	}
	atalk.client_sockfd = sockfd;

	return 0;
}

static void hub_destory(void)
{
	close(atalk.server_sockfd);
	close(atalk.client_sockfd);
	atalk.server_sockfd = -1;
	atalk.client_sockfd = -1;
	unlink(HOST_PATH);
}

static inline int hub_recv(char *buffer, size_t len)
{
	return recv(atalk.server_sockfd, buffer, len, 0);
}

static inline int hub_send(char *buffer, size_t len)
{
	return sendto(atalk.client_sockfd, buffer, len, 0,
		      (struct sockaddr *)&atalk.client_addr,
		      sizeof(struct sockaddr_un));
}

#define send_result_obj(cmd, result_obj)	\
	send_obj(cmd, NULL, result_obj)
#define send_notification_obj(method, method_obj)	\
	send_obj(NULL, method, method_obj)
static int send_obj(json_object *cmd, char *method, json_object *obj)
{
	const char *s;
	json_object *o, *m, *id;

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
	hub_send((char *)s, strlen(s));

	json_object_put(o);

	return 0;
}

enum player_state {
	player_play_state = 0,
	player_pause_state,
	player_stop_state,
};

static int send_player_state_change(enum player_state state)
{
	json_object *params;

	params = json_object_new_object();
	json_object_object_add(params, "state", json_object_new_int(state));

	if (send_notification_obj("player_state_change", params)) {
		json_object_put(params);
		return -1;
	}

	return 0;
}

enum wifi_state {
	wifi_none = -1,
	wifi_start,
	wifi_end_ok,
	wifi_end_fail,
};

static int send_wifi_state_change(enum wifi_state state)
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

static int __send_play_done(const char *url, int error_no)
{
	json_object *params, *error = NULL;

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

	atalk.play_state = ATALK_PLAY_IDLE;

	return 0;
}

static int send_player_volume_change(int volume)
{
	json_object *params;

	params = json_object_new_object();
	json_object_object_add(params, "volume", json_object_new_int(volume));

	if (send_notification_obj("player_volume_change", params)) {
		json_object_put(params);
		return -1;
	}

	return 0;
}

static int send_button_event(char *name, char *str, char *value)
{
	json_object *params;

	params = json_object_new_object();
	json_object_object_add(params, "name", json_object_new_string(name));
	if (str && value)
		json_object_object_add(params, str, json_object_new_string(value));

	if (send_notification_obj("button", params)) {
		json_object_put(params);
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * handler
 *******************************************************************************/
static bool atalk_is_attaching(json_object *cmd)
{
	return __atalk_is_attaching();
}

static int play_handler(json_object *cmd)
{
	const char *url;
	int show_url = 0;
	json_object *params, *uri, *artist, *title, *vendor;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;
	if (!json_object_object_get_ex(params, "uri", &uri))
		return -1;
	if (!json_object_object_get_ex(params, "title", &title))
		return -1;
	if (!json_object_object_get_ex(params, "artist", &artist))
		return -1;
	if (!json_object_object_get_ex(params, "vendor", &vendor))
		return -1;

	url = json_object_get_string(uri);

	pthread_mutex_lock(&atalk_state_mutex);
	if (!__atalk_is_online())
		printf("[Warning] %s: state_map = %d\n", __func__, atalk.state_map);
	if (!__atalk_is_attaching())
		__switch_mode_handler(true);
	else
		share_mem_set(LOCALPLAYER_DOMAIN, STATUS_STOPPING);
	if (share_mem_set(ATALK_DOMAIN, STATUS_PLAYING))
		printf("%s: share_mem_set fail.\n", __func__);
	atalk.play_state = ATALK_PLAY_RUN;
	send_player_state_change(player_play_state);

	printf("    url: %s\n", url);
	if (url[0] == '/' && access(url, R_OK)) {
		__send_play_done(url, 0);
	} else if (!__atalk_is_online()) {
		__send_play_done(url, 0);
	} else {
		if (mozart_player_playurl(atalk_player_handler, (char *)url))
			printf("[Warning] %s: mozart_player_playurl fail\n", __func__);
		free(current_url);
		current_url = strdup(url);
		show_url = 1;
	}
	/* TODO */
	/* if (atalk.play_state == ATALK_PLAY_PAUSE) */
	pthread_mutex_unlock(&atalk_state_mutex);

	if (show_url)
		mozart_smartui_atalk_play((char *)json_object_get_string(vendor),
					  (char *)json_object_get_string(title),
					  (char *)json_object_get_string(artist),
					  play_prompt);

	return 0;
}

static void __stop_handler(void)
{
	send_player_state_change(player_stop_state);

	if (atalk.play_state == ATALK_PLAY_IDLE)
		return ;
	/* If ATALK_DOMAIN set STOPPING, will call switch_mode. */
	if (share_mem_set(ATALK_DOMAIN, STATUS_PAUSE))
		printf("%s: share_mem_set fail.\n", __func__);
	atalk.play_state = ATALK_PLAY_IDLE;

	if (mozart_player_stop(atalk_player_handler))
		printf("[Warning] %s: mozart_player_stop fail\n", __func__);
}

static int stop_handler(json_object *cmd)
{
	pthread_mutex_lock(&atalk_state_mutex);
	__stop_handler();
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

static void __pause_handler(void)
{
	if (share_mem_set(ATALK_DOMAIN, STATUS_PAUSE))
		printf("%s: share_mem_set fail.\n", __func__);
	atalk.play_state = ATALK_PLAY_PAUSE;
	send_player_state_change(player_pause_state);

	if (mozart_smartui_is_atalk_view())
		mozart_smartui_atalk_toggle(false);
	else
		printf("[Warning] %s: smartui is %s\n", __func__, mozart_smartui_owner());

	if (mozart_player_pause(atalk_player_handler))
		printf("[Warning] %s: mozart_player_pause fail\n", __func__);
}

static int pause_handler(json_object *cmd)
{
	pthread_mutex_lock(&atalk_state_mutex);
	__pause_handler();
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

static void __resume_handler(void)
{
	if (share_mem_set(ATALK_DOMAIN, STATUS_PLAYING))
		printf("%s: share_mem_set fail.\n", __func__);
	atalk.play_state = ATALK_PLAY_PLAY;
	send_player_state_change(player_play_state);

	if (mozart_smartui_is_atalk_view())
		mozart_smartui_atalk_toggle(true);
	else
		printf("[Warning] %s: smartui is %s\n", __func__, mozart_smartui_owner());

	if (mozart_player_resume(atalk_player_handler))
		printf("[Warning] %s: mozart_player_resume fail\n", __func__);
}

static int resume_handler(json_object *cmd)
{
	pthread_mutex_lock(&atalk_state_mutex);
	if (!__atalk_is_online())
		printf("[Warning] %s: state_map = %d\n", __func__, atalk.state_map);
	if (!__atalk_is_attaching()) {
		__switch_mode_handler(true);
		/* FIXME */
		__send_play_done("NULL", 0);
	}

	__resume_handler();
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

static int pause_toggle_handler(json_object *cmd)
{
	pthread_mutex_lock(&atalk_state_mutex);
	if (atalk.play_state == ATALK_PLAY_PAUSE)
		__resume_handler();
	else if (atalk.play_state == ATALK_PLAY_RUN ||
		 atalk.play_state == ATALK_PLAY_PLAY)
		__pause_handler();
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

static int set_volume_handler(json_object *cmd)
{
	int vol;
	json_object *params, *volume;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;

	if (!json_object_object_get_ex(params, "volume", &volume))
		return -1;

	vol = json_object_get_int(volume);
	if (vol == 1)
		vol = 0;

	mozart_volume_set(vol, MUSIC_VOLUME);

	return 0;
}



static int play_voice_prompt_handler(json_object *cmd)
{
	module_status status;
	char mp3_src[256] = {0};
	bool is_playing = false, need_stop_other = false;
	const char *url;
	json_object *params, *uri;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;
	if (!json_object_object_get_ex(params, "uri", &uri))
		return -1;

	url = json_object_get_string(uri);

	if (!play_tone_get_source(mp3_src, "atalk_entry_13") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("栏目订阅");
		if (mozart_smartui_is_atalk_view())
			mozart_smartui_atalk_prompt(play_prompt);
		need_stop_other = true;
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_14") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("音乐电台");
		if (mozart_smartui_is_atalk_view())
			mozart_smartui_atalk_prompt(play_prompt);
		need_stop_other = true;
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_15") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("随便听听");
		if (mozart_smartui_is_atalk_view())
			mozart_smartui_atalk_prompt(play_prompt);
		need_stop_other = true;
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_16") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("广播电台");
		if (mozart_smartui_is_atalk_view())
			mozart_smartui_atalk_prompt(play_prompt);
		need_stop_other = true;
	} else if (!play_tone_get_source(mp3_src, "atalk_entry_17") && !strcmp(url, mp3_src)) {
		free(play_prompt);
		play_prompt = strdup("音乐收藏");
		if (mozart_smartui_is_atalk_view())
			mozart_smartui_atalk_prompt("音乐收藏");
		need_stop_other = true;
	} else if (!play_tone_get_source(mp3_src, "atalk_hi_12") && !strcmp(url, mp3_src)) {
		mozart_smartui_boot_welcome();
		need_stop_other = true;
	}

	pthread_mutex_lock(&atalk_state_mutex);
	if (!__atalk_is_online() && need_stop_other) {
		pr_debug("[Warning] %s: Don't play %s\n", __func__, url);
	} else {
		if (atalk.play_state == ATALK_PLAY_RUN ||
		    atalk.play_state == ATALK_PLAY_PLAY)
			is_playing = true;

		if (share_mem_get(ATALK_DOMAIN, &status))
			printf("%s: share_mem_get fail!\n", __func__);

		if (is_playing)
			__pause_handler();
		else if (status == STATUS_RUNNING)
			share_mem_stop_other(ATALK_DOMAIN);

		if (!strncmp(url, "file://", 7))
			mozart_play_tone_sync((char *)url + 7);
		else
			mozart_play_tone_sync((char *)url);

		if (is_playing)
			__resume_handler();
	}
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

static int net_state_change_handler(json_object *cmd)
{
	json_object *params, *state;

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;

	if (!json_object_object_get_ex(params, "state", &state))
		return -1;

	if (json_object_get_int(state) == 0) {
		mozart_event event = {
			.event = {
				.misc = {
					.name = "atalk",
					.type = "offline",
				},
			},
			.type = EVENT_MISC,
		};

		stop_handler(cmd);
		mozart_event_send(event);
	} else {
		mozart_event event = {
			.event = {
				.misc = {
					.name = "atalk",
					.type = "online",
				},
			},
			.type = EVENT_MISC,
		};

		mozart_event_send(event);
	}

	return 0;
}

static int update_cache_handler(json_object *cmd)
{
	struct update_msg *msg;
	json_object *params, *uri, *title, *artist, *vendor;

	if (up_die) {
		printf("[Error] %s. Download function not work\n", __func__);
		return -1;
	}

	if (!json_object_object_get_ex(cmd, "params", &params))
		return -1;
	if (!json_object_object_get_ex(params, "uri", &uri))
		return -1;
	if (!json_object_object_get_ex(params, "title", &title))
		return -1;
	if (!json_object_object_get_ex(params, "artist", &artist))
		return -1;
	if (!json_object_object_get_ex(params, "vendor", &vendor))
		return -1;

	msg = malloc(sizeof(struct update_msg));
	if (!msg) {
		printf("[Error] %s. Alloc message: %s\n", __func__, strerror(errno));
		return -1;
	}

	msg->uri = strdup(json_object_get_string(uri));
	msg->title = strdup(json_object_get_string(title));
	msg->artist = strdup(json_object_get_string(artist));
	if (!msg->uri || !msg->title || !msg->artist) {
		printf("[Error] %s. uri or title: %s\n", __func__, strerror(errno));
		goto err_node_str;
	}

	pthread_mutex_lock(&q_lock);
	if (up_die) {
		pthread_mutex_unlock(&q_lock);
		goto out_up_die;
	}
	/* Insert at queue tail */
	list_insert_at_tail(&atalk.up_queue_list, msg);
	pthread_mutex_unlock(&q_lock);

	pthread_mutex_lock(&cond_lock);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&cond_lock);

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

static int is_attaching_handler(json_object *cmd)
{
	json_object *result = json_object_new_object();

	pthread_mutex_lock(&atalk_state_mutex);
	if (__atalk_is_attaching())
		json_object_object_add(result, "attach", json_object_new_string("true"));
	else
		json_object_object_add(result, "attach", json_object_new_string("false"));
	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		pthread_mutex_unlock(&atalk_state_mutex);
		return -1;
	} else {
		pthread_mutex_unlock(&atalk_state_mutex);
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

	pthread_mutex_lock(&atalk_state_mutex);
	if (__atalk_is_online())
		json_object_object_add(result, "state", json_object_new_int(1));
	else if (__atalk_is_net())
		json_object_object_add(result, "state", json_object_new_int(0));
	else
		json_object_object_add(result, "state", json_object_new_int(0));
	if (send_result_obj(cmd, result)) {
		json_object_put(result);
		pthread_mutex_unlock(&atalk_state_mutex);
		return -1;
	} else {
		pthread_mutex_unlock(&atalk_state_mutex);
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

static int __switch_mode_handler(bool attach)
{
	int ret = 0;

	if (attach)
		__atalk_set_attaching();
	else
		__atalk_clear_attaching();

	if (__atalk_app_is_stop())
		return -1;

	if (attach) {
		__atalk_prepare_play();
		ret = send_button_event("switch_mode", "attach", "true");
	} else {
		ret = send_button_event("switch_mode", "attach", "false");
		__atalk_stop_play();

		mozart_play_key_sync("atalk_swich_false_5");
	}

	return ret;
}

static struct atalk_method methods[] = {
	/* notification */
	{
		.name = "play",
		.handler = play_handler,
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
		.is_valid = atalk_is_attaching,
	},
	{
		.name = "play_voice_prompt",
		.handler = play_voice_prompt_handler,
	},
	{
		.name = "net_state_change",
		.handler = net_state_change_handler,
	},
	{
		.name = "update_cache",
		.handler = update_cache_handler,
	},
	/* TODO: set_led, update_screen */

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

static void *atalk_cli_func(void *args)
{
	char cmd[512];

	while (1) {
		int i;
		const char *method;
		json_object *c, *o;
		bool is_valid = true;
		struct atalk_method *m;

		memset(cmd, 0, sizeof(cmd));
		hub_recv(cmd, sizeof(cmd));
		pr_debug(">>>> %s: Recv: %s\n", __func__, cmd);

		c = json_tokener_parse(cmd);
		json_object_object_get_ex(c, "method", &o);
		method = json_object_get_string(o);

		for (i = 0; i < ARRAY_SIZE(methods); i++) {
			m = &methods[i];
			if (!strcmp(m->name, method)) {
				if (m->is_valid)
					is_valid = m->is_valid(c);
				if (is_valid)
					m->handler(c);
				else
					pr_debug("     %s invalid\n", cmd);
				break;
			}
		}

		if (i >= ARRAY_SIZE(methods))
			printf("%s: invalid command: %s\n", __func__, method);

		json_object_put(c);
	}

	hub_destory();

	return NULL;
}

static int atalk_player_status_callback(player_snapshot_t *snapshot,
					struct player_handler *handler, void *param)
{
	if (strcmp(handler->uuid, snapshot->uuid))
		return 0;

	pr_debug("%s: status = %d, %s\n", __func__,
		 snapshot->status, player_status_str[snapshot->status]);

	/* TODO: error? */
	pthread_mutex_lock(&atalk_state_mutex);
	switch (snapshot->status) {
	case PLAYER_TRANSPORT:
	case PLAYER_PLAYING:
		atalk.play_state = ATALK_PLAY_PLAY;
		break;
	case PLAYER_PAUSED:
		break;
	case PLAYER_UNKNOWN:
		break;
	case PLAYER_STOPPED:
		__send_play_done(current_url, 0);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

void end_func(DPres_t res, char *errStr, void *userData)
{
	int *result = (int *)userData;

	switch (res) {
	case DP_OK:
		*result = 0;
		break;
	default:
		printf("[ERROR].atalk download: %s\n", errStr);
		*result = -1;
	}
}

#define TIMEOUT 4
static int atalk_update_download(const char *uri, const char *save)
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

	atalk.dp->endFunc	= end_func;
	atalk.dp->endData	= &res;
	atalk.dp->maxSpeedLimit = 64 * 1024; /* Limit download speed 64KB */

	err = dl_perform_sync(atalk.dp, (char *)uri, tempfile, 0);
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

static int atalk_update_cache_clear(const char *dir, char *key)
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

static int atalk_queue_head(const void *data, const void *key)
{
	return 0;
}

static void atalk_list_destroy(void *data)
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

static void *atalk_update_queue_handle_func(void *data)
{
	char dir_target[] = {"/mnt/sdcard/atalk-favorite"};
	char *path_title = NULL;
	int title_size;
	struct stat st;
	module_status status;
	struct update_msg *msg;
	int scan_flag = 0;
	int err;

	/* Check SDCARD */
	if (share_mem_get(SDCARD_DOMAIN, &status)) {
		printf("%s: share_mem_get fail!\n", __func__);
		goto err_pre;
	}
	if (status != STATUS_INSERT) {
		printf("[Error] %s. No SDCARD\n", __func__);
		goto err_pre;
	}

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
		atalk_update_cache_clear(dir_target, ".ucache");
	}

	while (!up_die) {
		while (!up_die && !is_empty(&atalk.up_queue_list)) {
			pthread_mutex_lock(&q_lock);
			/* Get queue at head */
			msg = list_delete(
				&atalk.up_queue_list,
				NULL,
				atalk_queue_head);
			pthread_mutex_unlock(&q_lock);

			if (strstr(msg->uri, "file://")) {
				printf("[Info] %s. %s is local file\n", __func__, msg->uri);
				goto up_file_pass;
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
				err = atalk_update_download(msg->uri, path_title);
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
	list_destroy(&atalk.up_queue_list, atalk_list_destroy);
	pthread_mutex_unlock(&q_lock);

err_pre:
	up_die = 1;

	return NULL;
}

/*******************************************************************************
 * interface
 *******************************************************************************/
int mozart_atalk_volume_set(int vol)
{
	mozart_volume_set(vol, MUSIC_VOLUME);
	send_player_volume_change(vol);

	return 0;
}

int mozart_atalk_prev(void)
{
	return send_button_event("previous", NULL, NULL);
}

int mozart_atalk_next(void)
{
	return send_button_event("next", NULL, NULL);
}

int mozart_atalk_next_channel(void)
{
	return send_button_event("next_channel", NULL, NULL);
}

int mozart_atalk_toggle(void)
{
	pr_debug("**** %s ****\n", __func__);
	return pause_toggle_handler(NULL);
}

int mozart_atalk_pause(void)
{
	enum atalk_play_state state;

	pthread_mutex_lock(&atalk_state_mutex);
	pr_debug("**** %s ****\n", __func__);
	state = atalk.play_state;
	if (state != ATALK_PLAY_RUN && state != ATALK_PLAY_PLAY) {
		/* TODO */
		/* if (state == ATALK_PLAY_IDLE) */
		pthread_mutex_unlock(&atalk_state_mutex);
		return 0;
	}
	__pause_handler();
	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

int mozart_atalk_is_control_mode(void)
{
	return __atalk_is_attaching();
}

int mozart_atalk_is_online(void)
{
	return __atalk_is_online();
}

int mozart_atalk_switch_mode(void)
{
	int ret;

	pthread_mutex_lock(&atalk_state_mutex);
	if (__atalk_is_attaching())
		ret = __switch_mode_handler(false);
	else
		ret = __switch_mode_handler(true);
	pthread_mutex_unlock(&atalk_state_mutex);

	return ret;
}

int mozart_atalk_love_audio(void)
{
	return send_button_event("love_audio", "uri", current_url);
}

int mozart_atalk_start(void)
{
	int ret = 0;

	pthread_mutex_lock(&atalk_state_mutex);

	pr_debug("**** %s ****\n", __func__);
	if (!__atalk_is_attaching())
		__switch_mode_handler(true);

	pthread_mutex_unlock(&atalk_state_mutex);

	return ret;
}

int mozart_atalk_stop_play(void)
{
	int ret = 0;

	pthread_mutex_lock(&atalk_state_mutex);

	pr_debug("**** %s ****\n", __func__);
	__atalk_stop_play();

	pthread_mutex_unlock(&atalk_state_mutex);

	return ret;
}

int mozart_atalk_stop(void)
{
	int ret = 0;

	pthread_mutex_lock(&atalk_state_mutex);

	pr_debug("**** %s ****\n", __func__);
	if (__atalk_is_attaching())
		__switch_mode_handler(false);

	pthread_mutex_unlock(&atalk_state_mutex);

	return ret;
}

/* true: success
 * false: fail
 */
int atalk_network_trigger(bool on)
{
	wifi_info_t wifi_info;

	pthread_mutex_lock(&atalk_state_mutex);

	if (__atalk_app_is_stop()) {
		pthread_mutex_unlock(&atalk_state_mutex);
		return -1;
	}

	wifi_info = get_wifi_mode();

	if (wifi_info.wifi_mode == STA || wifi_info.wifi_mode == STANET) {
		pr_debug("**** %s: wifi sta mode (state:%s_%s, play: %s) ****\n",
			 __func__, __atalk_is_attaching() ? "ATTACH" : "UNATTACH",
			 __atalk_is_online() ? "ONLINE" : "OFFLINE",
			 atalk_play_state_str[atalk.play_state]);

		if (on) {
			if (!__atalk_is_online())
				__atalk_set_online();
			if (__atalk_is_net()) {
				__atalk_set_net_over();
				send_wifi_state_change(wifi_end_ok);
			}
		}

		__atalk_prepare_play();
		atalk_dump();
	} else if (wifi_info.wifi_mode == AIRKISS) {
		pr_debug("**** %s: wifi airkiss mode (state:%s_%s, play: %s) ****\n",
			 __func__, __atalk_is_attaching() ? "ATTACH" : "UNATTACH",
			 __atalk_is_online() ? "ONLINE" : "OFFLINE",
			 atalk_play_state_str[atalk.play_state]);

		if (on) {
			__atalk_set_offline();
			__atalk_set_net();
			send_wifi_state_change(wifi_start);
		}

		__atalk_stop_play();
		atalk_dump();
	} else if (wifi_info.wifi_mode == AP) {
		pr_debug("**** %s: wifi ap mode (state:%s_%s, play: %s) ****\n",
			 __func__, __atalk_is_attaching() ? "ATTACH" : "UNATTACH",
			 __atalk_is_online() ? "ONLINE" : "OFFLINE",
			 atalk_play_state_str[atalk.play_state]);

		if (on) {
			__atalk_set_offline();
			if (!__atalk_is_net()) {
				send_wifi_state_change(wifi_start);
				send_wifi_state_change(wifi_end_fail);
			} else {
				send_wifi_state_change(wifi_end_fail);
			}
		}

		__atalk_stop_play();
		atalk_dump();
	}

	pthread_mutex_unlock(&atalk_state_mutex);

	return 0;
}

int start_atalk_app(void)
{
	pthread_t atalk_thread;

	if (!atalk_initialized) {
		if (hub_init())
			return -1;

		if (share_mem_init()) {
			printf("%s: share_mem_init fail\n", __func__);
			return -1;
		}

		atalk_player_handler =
			mozart_player_handler_get("atalk", atalk_player_status_callback, NULL);
		if (atalk_player_handler == NULL) {
			printf("%s: get_player_handler fail!\n", __func__);
			return -1;
		}

		if (pthread_create(&atalk_thread, NULL, atalk_cli_func, NULL) != 0) {
			printf("%s: Can't create atalk_thread: %s\n",
			       __func__, strerror(errno));
			return -1;
		}
		pthread_detach(atalk_thread);

		up_die = 0;
		list_init(&atalk.up_queue_list);
		if (pthread_create(&atalk.down_thread, NULL, atalk_update_queue_handle_func, NULL) != 0) {
			printf("%s: Can't create down_thread: %s\n",
				__func__, strerror(errno));
			return -1;
		}

		atalk.dp = dl_perform_init();
		if (!atalk.dp)
			return -1;

		/* FIXME */
		mozart_system("date 2015-12-12");
		atalk_initialized = true;
	}

#ifdef ATALK_CTL_DEBUG
	if (!access("/mnt/sdcard/atalk_vendor_log.txt", R_OK | W_OK))
		mozart_system("sleep 8 && atalk_vendor -c /usr/fs/etc/atalk/prodconf.json >"
			      "/mnt/sdcard/atalk_vendor_log.txt 2>&1 &");
	else
#endif
		mozart_system("sleep 8 && atalk_vendor -c /usr/fs/etc/atalk/prodconf.json >/dev/null 2>&1 &");
	if (share_mem_set(ATALK_DOMAIN, STATUS_RUNNING))
		printf("%s: share_mem_set fail.\n", __func__);

	atalk_network_trigger(true);

	return 0;
}

void stop_atalk_app(void)
{
	pthread_mutex_lock(&atalk_state_mutex);
	if (__atalk_app_is_stop()) {
		pthread_mutex_unlock(&atalk_state_mutex);
		return ;
	}

	free(play_prompt);
	play_prompt = NULL;
	free(current_url);
	current_url = NULL;

	__atalk_set_attaching();
	__atalk_set_offline();
	__atalk_stop_play();
	if (share_mem_set(ATALK_DOMAIN, STATUS_SHUTDOWN))
		printf("%s: share_mem_set fail\n", __func__);

	up_die = 1;
	dl_perform_stop(atalk.dp);

	pthread_mutex_lock(&cond_lock);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&cond_lock);

	pthread_join(atalk.down_thread, NULL);
	dl_perform_uninit(atalk.dp);

	mozart_system("killall atalk_vendor");
	unlink("/var/run/doug.pid");
	pthread_mutex_unlock(&atalk_state_mutex);
}
