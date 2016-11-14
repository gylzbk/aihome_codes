/*
 * copy from mozart/src/main/airplay.c
 * */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "player_interface.h"
#include "sharememory_interface.h"

#define AIRPLAY_COMMAND_STOP	'4'
#define AIRPLAY_COMMAND_PAUSE	'5'
#define AIRPLAY_COMMAND_RESUME	'6'
#define AIRPLAY_COMMAND_EXIT	'7'

#define AIRPLAY_SERVER	"127.0.0.1"
#define AIRPLAY_PORT	13578

#if 0
static int connect_to_tcp_server(char *ipaddr, int port)
{
	int sockfd = -1;
	struct sockaddr_in seraddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd) {
		printf("socket() error for connect to shairport failed: %s.\n", strerror(errno));
		return -1;
	}

	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(port);
	seraddr.sin_addr.s_addr = inet_addr(ipaddr);

	if (connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr))) {
		printf("connect() error for connect to shairport failed: %s.\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	return sockfd;
}

static int mozart_airplay_control(char cmd)
{
	int sockfd = -1;
	ssize_t wSize, rSize;
	char res;
	int err = 0;

	sockfd = connect_to_tcp_server(AIRPLAY_SERVER, AIRPLAY_PORT);
	if (sockfd < 0) {
		printf("connect to airplay error.\n");
		return -1;
	}

	wSize = send(sockfd, &cmd, 1, 0);
	if (wSize != 1) {
		printf("send stop command: %s\n", strerror(errno));
		err = -1;
		goto err_send;
	}

	rSize = recv(sockfd, &res, 1, 0);
	if (rSize != 1) {
		printf("recv res: %s\n", strerror(errno));
		err = -1;
	}

err_send:
	close(sockfd);

	return err;
}

static int mozart_airplay_shutdown(void)
{
	return mozart_airplay_control(AIRPLAY_COMMAND_EXIT);
}

void stop_sound(void)
{
	player_handler_t *handler = NULL;
	player_status_t status = PLAYER_UNKNOWN;
	module_status airplay_domain_status;

	handler = mozart_player_handler_get("updater", NULL, NULL);
	if (handler) {
		status = mozart_player_getstatus(handler);
		if (status == PLAYER_PLAYING || status == PLAYER_PAUSED)
			mozart_player_stopall(handler, true);
	}

	share_mem_init();
	share_mem_get(AIRPLAY_DOMAIN, &airplay_domain_status);
	if (airplay_domain_status == STATUS_PLAYING || airplay_domain_status == STATUS_PAUSE)
		mozart_airplay_shutdown();

	return;
}
#else
void stop_sound(void)
{
	return;
}
#endif
