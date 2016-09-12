#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils_interface.h"
#include "tips_interface.h"
#include "sharememory_interface.h"

#include "mozart_module.h"

#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#endif

#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"

#ifndef MOZART_RELEASE
#define MOZART_AIRPLAY_DEBUG
#else
#define MOZART_RELEASE_NAME
#endif

#ifdef MOZART_AIRPLAY_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[AIRPLAY] %s: "fmt, __func__, ##args)
#else  /* MOZART_AIRPLAY_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_AIRPLAY_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[AIRPLAY] [Error] %s: "fmt, __func__, ##args)

static char *cmdline[32];
static char apname[64];
static char ao_type[10];

#define AIRPLAY_COMMAND_STOP	'4'
#define AIRPLAY_COMMAND_PAUSE	'5'
#define AIRPLAY_COMMAND_RESUME	'6'

#define AIRPLAY_RES_SYNC	'0'
static int mozart_airplay_control(char cmd);
/*******************************************************************************
 * module
 *******************************************************************************/
static int airplay_module_start(struct mozart_module_struct *self)
{
	int i;
	module_status domain_status;

	/* wait 0.5s */
	for (i = 0; i < 10; i++) {
		if (share_mem_get(AIRPLAY_DOMAIN, &domain_status)) {
			pr_err("share_mem_get failed!\n");
			return -1;
		}

		if (domain_status != WAIT_RESPONSE)
			usleep(50 * 1000);
		else
			break;
	}

	if (i >= 10)
		pr_err("wait WAIT_REPONSE timeout!\n");

	if (self->module_change)
		__mozart_prompt_tone_key_sync("atalk_swich_false_5");
	mozart_smartui_linein_play();
	self->player_state = player_state_idle;

	share_mem_set(AIRPLAY_DOMAIN, RESPONSE_DONE);

	return 0;
}

static int airplay_module_run(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play)
		return mozart_airplay_control(AIRPLAY_COMMAND_RESUME);
	else if (self->player_state == player_state_idle)
		self->player_state = player_state_play;

	return 0;
}

static int airplay_module_suspend(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play)
		return mozart_airplay_control(AIRPLAY_COMMAND_PAUSE);
	else
		return 0;
}

static int airplay_module_stop(struct mozart_module_struct *self)
{
	self->player_state = player_state_idle;
	mozart_airplay_control(AIRPLAY_COMMAND_STOP);

	return 0;
}

static void airplay_module_resume_pause(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (self->player_state == player_state_play) {
		/* pause */
		mozart_airplay_control(AIRPLAY_COMMAND_PAUSE);
		self->player_state = player_state_pause;
	} else if (self->player_state == player_state_pause) {
		/* resume */
		mozart_airplay_control(AIRPLAY_COMMAND_RESUME);
		self->player_state = player_state_play;
	}

	mozart_module_mutex_unlock();
}

static void airplay_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();
	#if (SUPPORT_VR == VR_ATALK)
		if (__mozart_module_is_online()){
			mozart_atalk_cloudplayer_start(true);
	  	} else {
			mozart_atalk_localplayer_start(true);
	  	}
	#elif (SUPPORT_VR == VR_SPEECH)
		if (__mozart_module_is_online()){
			mozart_aitalk_cloudplayer_start(true);
	  	} else {
			mozart_aitalk_localplayer_start(true);
	  	}
	#endif
	mozart_module_mutex_unlock();
}

static struct mozart_module_struct airplay_module = {
	.name = "airplay",
	.priority = 1,
	.attach = module_unattach,
	.mops = {
		.on_start   = airplay_module_start,
		.on_run     = airplay_module_run,
		.on_suspend = airplay_module_suspend,
		.on_stop    = airplay_module_stop,
	},
	.kops = {
		.resume_pause = airplay_module_resume_pause,
		.next_module = airplay_module_next_module,
	},
};

/*******************************************************************************
 * Function
 *******************************************************************************/
static int connect_to_tcp_server(char *ipaddr, int port)
{
	int sockfd = -1;
	struct sockaddr_in seraddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd) {
		pr_err("socket() error for connect to shairport failed: %s.\n", strerror(errno));
		return -1;
	}

	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(port);
	seraddr.sin_addr.s_addr = inet_addr(ipaddr);

	if (connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr))) {
		pr_err("connect() error for connect to shairport failed: %s.\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	return sockfd;
}

#define AIRPLAY_SERVER	"127.0.0.1"
#define AIRPLAY_PORT	13578
static int mozart_airplay_control(char cmd)
{
	int sockfd = -1;
	ssize_t wSize, rSize;
	char res;
	int err = 0;

	sockfd = connect_to_tcp_server(AIRPLAY_SERVER, AIRPLAY_PORT);
	if (sockfd < 0) {
		pr_err("connect to airplay error.\n");
		return -1;
	}

	wSize = send(sockfd, &cmd, 1, 0);
	if (wSize != 1) {
		pr_err("send stop command: %s\n", strerror(errno));
		err = -1;
		goto err_send;
	}

	rSize = recv(sockfd, &res, 1, 0);
	if (rSize != 1) {
		pr_err("recv res: %s\n", strerror(errno));
		err = -1;
	}

err_send:
	close(sockfd);

	return err;
}

static int mozart_airplay_init(void)
{
	int i = 0;
	int argc = 0;
	__attribute__((unused)) char mac[] = "00:11:22:33:44:55";

	strcpy(ao_type, "oss");
	if (AUDIO_ALSA == get_audio_type())
		strcpy(ao_type, "alsa");

	char *argv[] = {
		"shairport",
		"-o",
		ao_type,
		"-a",
		apname,
#if 0
		"-e",
		"/var/log/shairport.errlog",
#endif
		"-d",
		NULL
	};

#ifdef MOZART_RELEASE_NAME
	memset(apname, 0, sizeof(apname));
	sprintf(apname, "DS-1825");
#else
	memset(mac, 0, sizeof(mac));
	memset(apname, 0, sizeof(apname));
	get_mac_addr("wlan0", mac, "");

	strcat(apname, "SmartAudio-");
	strcat(apname, mac+4);
#endif

	argc = sizeof(argv) / sizeof(argv[0]);

	for (i = 0; i < argc; i++)
		cmdline[i] = argv[i];

	cmdline[0] = "shairport";
	cmdline[argc] = NULL;

	return 0;
}

static void mozart_airplay_start_service(void)
{
	int i = 0;
	char cmd[128] = {};

#define CMDLINE_DEBUG 0
#if CMDLINE_DEBUG
	while (cmdline[i++])
		printf("argv[%d] is %s\n", i - 1, cmdline[i - 1]);
#endif

#if 0
	if (0 == fork())
		execvp(cmdline[0], cmdline);
#else
	i = 0;
	while (cmdline[i]) {
		strcat(cmd, cmdline[i]);
		strcat(cmd, " ");
		i++;
	}

#if CMDLINE_DEBUG
	printf("shairport cmd is `%s`\n", cmd);
#endif

	mozart_system(cmd);
#endif

	return;
}

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_airplay_is_start(void)
{
	return __mozart_module_is_start(&airplay_module);
}

int mozart_airplay_start(bool in_lock)
{
	if (airplay_module.start) {
		return airplay_module.start(&airplay_module, module_cmd_stop, in_lock);
	} else {
		pr_err("airplay_module isn't registered!\n");
		return -1;
	}
}

int mozart_airplay_startup(void)
{
	if (mozart_module_register(&airplay_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	mozart_airplay_init();
	mozart_airplay_start_service();

	return 0;
}

int mozart_airplay_shutdown(void)
{
	if (airplay_module.stop && __mozart_airplay_is_start())
		airplay_module.stop(&airplay_module, module_cmd_stop, false);
	if (!mozart_module_unregister(&airplay_module))
		system("killall shairport");

	return 0;
}
