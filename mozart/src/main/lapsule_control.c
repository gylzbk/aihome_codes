#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "utils_interface.h"
#include "sharememory_interface.h"
#if (SUPPORT_DMR == 1)
#include "render_interface.h"
#endif
#include "lapsule_control.h"

#define DEBUG(x,y...)	{printf("[ %s : %s : %d] ",__FILE__, __func__, __LINE__); printf(x,##y); printf("\n");}
#define ERROR(x,y...)	{printf("[ %s : %s : %d] ",__FILE__, __func__, __LINE__); printf(x,##y); printf("\n");}

#define LAPSULE_SOCK_ADDR "127.0.0.1"
#define LAPSULE_SOCK_PORT 10098

#define LAPSULE_PLAY "{\"command\": \"play\"}"
#define LAPSULE_STOP "{\"commands\": [{\"command\": \"set_play_next\",\"is_play_next\": false},{\"command\": \"stop\"}]}"
#define LAPSULE_WAKEUP "{\"commands\": [{\"command\": \"set_play_next\",\"is_play_next\": true},{\"command\": \"play\"}]}"
#define LAPSULE_PAUSE "{\"command\": \"pause\"}"
#define LAPSULE_TOGGLE "{\"command\": \"toggle\"}"
#define LAPSULE_SKIP "{\"command\": \"skip\"}"
#define LAPSULE_PREV "{\"command\": \"prefix\"}"
#define LAPSULE_SET_VOLUME "{\"command\": \"set_volume\", \"volume\": %d}"
#define LAPSULE_GET_VOLUME "{\"command\": \"get_volume\"}"
#define LAPSULE_SEARCH_SONG "{\"command\": \"search\", \"keyword\": \"%s\"}"
#define LAPSULE_MUSIC_LIST "{\"command\": \"play_default_channel\",\"index\": %d}"
#define LAPSULE_NOTIFY_LINEIN_ON "{\"command\": \"set_linein_mode\",\"mode\": 1}"
#define LAPSULE_NOTIFY_LINEIN_OFF "{\"command\": \"set_linein_mode\",\"mode\": 0}"
#define RESULT_SIZE 128

static int lapsule_socket = -1;

static void close_lapsule_socket(void)
{
	if(lapsule_socket > 0){
		close(lapsule_socket);
		lapsule_socket = -1;
	}
}

int start_lapsule_app(void)
{
	if(mozart_system("lapsule -a &")) {
        printf("start lapsule services failed in %s: %s.\n", __func__, strerror(errno));
        return errno;
    }

    return 0;
}

void stop_lapsule_app(void)
{
#if (SUPPORT_DMR == 1)
	control_point_info *info = NULL;
	memory_domain domain;
#endif

#if (SUPPORT_DMR == 1)
	ret = share_mem_get_active_domain(&domain);
	if(0 == ret && RENDER_DOMAIN == domain) {
		info = mozart_render_get_controler();
		if(strcmp(LAPSULE_PROVIDER, info->music_provider) == 0){		//is lapsule in control
			lapsule_do_stop_play();
		}
	}
#endif
        close_lapsule_socket();

	mozart_system("killall lapsule");
	mozart_system("rm -f /mnt/vendor/lapsule_dist/db/is_restart");
}

static int connect_to_lapsule(void)
{
	int sockfd = -1;
	struct sockaddr_in lapsule_addr;

	if(lapsule_socket > -1){
		return lapsule_socket;
	}

	if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0))){
		ERROR("socket:%s", strerror(errno));
		return -1;
	}

	lapsule_addr.sin_family = AF_INET;
	lapsule_addr.sin_port = htons(LAPSULE_SOCK_PORT);
	lapsule_addr.sin_addr.s_addr = inet_addr(LAPSULE_SOCK_ADDR);
	if (-1 == connect(sockfd, (struct sockaddr *)&lapsule_addr, sizeof(lapsule_addr))){
		ERROR("connect:%s", strerror(errno));
		close(sockfd);
		return -1;
	}

	return sockfd;
}

static int send_command(char *command)
{
	int rs = 0;
	rs = send(lapsule_socket, command, strlen(command) + 1, 0);
	if(rs == strlen(command) + 1){
		return 0;
	}
	ERROR("send:%s", strerror(errno));
	if(EBADF == errno || ENOTCONN == errno || ENOTSOCK == errno){
		return -2;
	}
	return -1;
}

int recv_result(char *result)
{
	if(-1 == recv(lapsule_socket, result, RESULT_SIZE, 0)){
		ERROR("recv:%s", strerror(errno));
		return -1;
	}
	return 0;
}

int lapsule_do_play(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_PLAY)){
        close_lapsule_socket();
        return -1;
    }

	return 0;
}

int lapsule_do_pause(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_PAUSE)){
        close_lapsule_socket();
        return -1;
	}

	return 0;
}

int lapsule_do_prev_song()
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_PREV)){
        close_lapsule_socket();
        return -1;
	}

	return 0;
}

int lapsule_do_next_song(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_SKIP)){
        close_lapsule_socket();
        return -1;
	}

    return 0;
}

int lapsule_do_toggle(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}
	if(0 != send_command(LAPSULE_TOGGLE)){
        close_lapsule_socket();
        return -1;
	}

    return 0;
}

int lapsule_do_stop_play(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_STOP)){
        close_lapsule_socket();
        return -1;
	}

    return 0;
}

int lapsule_do_wakeup(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	if(0 != send_command(LAPSULE_WAKEUP)){
        close_lapsule_socket();
        return -1;
	}

	return 0;
}

int lapsule_do_set_vol(int vol)
{
	char cmd[128] = {0};

	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	sprintf(cmd, LAPSULE_SET_VOLUME, vol);
	if(0 != send_command(cmd)){
        close_lapsule_socket();
        return -1;
	}

	return 0;
}

int lapsule_do_search_song(char *song)
{
	char cmd[128] = {0};

	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
        return -1;
	}

	sprintf(cmd, LAPSULE_SEARCH_SONG, song);
	if(0 != send_command(cmd)){
        close_lapsule_socket();
        return -1;
	}

	return 0;
}

int lapsule_do_music_list(int keyIndex)
{
	char cmd[128] = {0};

	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
		return -1;
	}

	sprintf(cmd, LAPSULE_MUSIC_LIST, keyIndex);
	printf("cmd is %s\n",cmd);
	if(0 != send_command(cmd)){
		close_lapsule_socket();
		return -1;
	}

	return 0;
}

int lapsule_do_linein_on(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
		return -1;
	}

	if(0 != send_command(LAPSULE_NOTIFY_LINEIN_ON)){
		close_lapsule_socket();
		return -1;
	}

	return 0;
}

int lapsule_do_linein_off(void)
{
	lapsule_socket = connect_to_lapsule();
	if(lapsule_socket < 0){
		ERROR("connect_to_lapsule failure.");
		return -1;
	}

	if(0 != send_command(LAPSULE_NOTIFY_LINEIN_OFF)){
		close_lapsule_socket();
		return -1;
	}

	return 0;
}
