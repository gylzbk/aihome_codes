#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <linux/soundcard.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "ini_interface.h"
#include "tips_interface.h"

#include "mozart_module.h"
#include "mozart_linein.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"


#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#include "vr-atalk_interface.h"
#include "mozart_atalk_cloudplayer_control.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#include "mozart_aitalk_cloudplayer_control.h"
#endif

#define SOUND_MIXER_WRITE_OUTSRC MIXER_WRITE(SOUND_MIXER_OUTSRC)

#ifndef MOZART_RELEASE
#define MOZART_LINEIN_DEBUG
#endif

#ifdef MOZART_LINEIN_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[LINEIN] %s: "fmt, __func__, ##args)
#else  /* MOZART_LINEIN_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_LINEIN_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[LINEIN] [Error] %s: "fmt, __func__, ##args)

static int linein_fd = -1;
static int mozart_linein_open(void);
static void mozart_linein_close(void);
/*******************************************************************************
 * module
 *******************************************************************************/
static int linein_module_start(struct mozart_module_struct *self)
{
	mozart_smartui_linein_play();
	if (self->module_change)
		__mozart_prompt_tone_key_sync("atalk_swich_false_5");
	self->player_state = player_state_idle;

	return 0;
}

static int linein_module_run(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play) {
		mozart_linein_open();
	} else if (self->player_state == player_state_idle) {
		mozart_linein_open();
		self->player_state = player_state_play;
	}

	return 0;
}

static int linein_module_suspend(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play)
		mozart_linein_close();

	return 0;
}


static int linein_module_stop(struct mozart_module_struct *self)
{
	self->player_state = player_state_idle;
	mozart_linein_close();

	return 0;
}

static void linein_module_resume_pause(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (self->player_state == player_state_play) {
		/* pause */
		mozart_linein_close();
		self->player_state = player_state_pause;
	} else if (self->player_state == player_state_pause) {
		/* resume */
		mozart_linein_open();
		self->player_state = player_state_play;
	}

	mozart_module_mutex_unlock();
}

static void linein_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (__mozart_module_is_online()){
		#if (SUPPORT_VR == VR_ATALK)
			mozart_atalk_cloudplayer_start(true);
		#elif (SUPPORT_VR == VR_ATALK)
			mozart_aitalk_cloudplayer_start(true);
		#endif

	}
	else
		mozart_atalk_localplayer_start(true);

	mozart_module_mutex_unlock();
}

static struct mozart_module_struct linein_module = {
	.name = "linein",
	.priority = 1,
	.attach = module_unattach,
	.mops = {
		.on_start   = linein_module_start,
		.on_run     = linein_module_run,
		.on_suspend = linein_module_suspend,
		.on_stop    = linein_module_stop,
	},
	.kops = {
		.resume_pause = linein_module_resume_pause,
		.next_module = linein_module_next_module,
	},
};

/*******************************************************************************
 * Function
 *******************************************************************************/
static bool mozart_m150_linein_is_in(void)
{
	char buf[2] = {};
	int fd = -1;

	fd = open("/sys/devices/platform/jz-linein/state", O_RDONLY);
	if (fd == -1) {
		printf("open /sys/devices/platform/jz-linein/state failed, can not get linein status.\n");
		return false;
	}

	if (read(fd, buf, 1) == 1) {
		close(fd);
		return buf[0] == '1' ? true : false;
	} else {
		close(fd);
		return false;
	}
}

static bool mozart_x1000_linein_is_in(void)
{
	char buf[2] = {};
	int fd = -1;

	fd = open("/sys/devices/virtual/switch/linein/state", O_RDONLY);
	if (fd == -1) {
		printf("open /sys/devices/virtual/switch/linein/state failed, can not get linein status.\n");
		return false;
	}

	if (read(fd, buf, 1) == 1) {
		close(fd);
		return buf[0] == '1' ? true : false;
	} else {
		close(fd);
		return false;
	}
}

static int mozart_m150_linein_open(void)
{
	linein_fd = open("/dev/linein", O_RDWR);
	if (linein_fd < 0) {
		printf("switch to linein mode fail, open /dev/linein error: %s", strerror(errno));
		return -1;
	}

	printf("[m150] switch to linein mode.\n");

	return linein_fd;
}

static int mozart_x1000_linein_open(void)
{
	int mode = SOUND_MIXER_LINE;
	int mixer_fd = -1;

	mixer_fd = open("/dev/mixer", O_RDWR);
	if (mixer_fd < 0) {
		perror("switch to linein mode fail, open /dev/mixer error");
		return -1;
	}

	if (ioctl(mixer_fd, SOUND_MIXER_WRITE_RECSRC, &mode) == -1) {
		perror("switch to linein mode failed, ioctl error");
		close(mixer_fd);
		return -1;
	}

	close(mixer_fd);
	printf("[x1000] switch to linein mode.\n");
	return 0;
}

static void mozart_m150_linein_close(void)
{
	if (linein_fd >= 0) {
		close(linein_fd);
		linein_fd = -1;
	}

	printf("[m150] exit linein mode, recovery origin router.\n");
}

static void mozart_x1000_linein_close(void)
{
	int mode = SOUND_MIXER_SPEAKER;
	int mixer_fd = -1;

	mixer_fd = open("/dev/mixer", O_RDWR);
	if (mixer_fd < 0) {
		perror("close linein mode fail, open /dev/mixer error");
		return;
	}

	if (ioctl(mixer_fd, SOUND_MIXER_WRITE_OUTSRC, &mode) == -1) {
		perror("switch to speaker mode, ioctl error");
		close(mixer_fd);
		return;
	}

	close(mixer_fd);
	printf("[x1000] exit linein mode, switch to speaker.\n");
	return;
}

static int mozart_linein_open(void)
{
	char buf[8] = {};

	if (mozart_ini_getkey("/usr/data/system.ini", "product", "cpu", buf))
		return mozart_m150_linein_open();

	if (!strcasecmp(buf, "x1000"))
		return mozart_x1000_linein_open();
	else if (!strcasecmp(buf, "m150"))
		return mozart_m150_linein_open();

	printf("Unknow cpu model, open linein failed.\n");
	return -1;
}

static void mozart_linein_close(void)
{
	char buf[8] = {};

	if (mozart_ini_getkey("/usr/data/system.ini", "product", "cpu", buf))
		return mozart_m150_linein_close();

	if (!strcasecmp(buf, "x1000"))
		return mozart_x1000_linein_close();
	else if (!strcasecmp(buf, "m150"))
		return mozart_m150_linein_close();

	printf("Unknow cpu model, close linein failed.\n");
	return;
}

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_linein_is_start(void)
{
	return __mozart_module_is_start(&linein_module);
}

bool mozart_linein_is_in(void)
{
	char buf[8] = {};
	if (mozart_ini_getkey("/usr/data/system.ini", "product", "cpu", buf))
		return mozart_m150_linein_is_in();

	if (!strcasecmp(buf, "x1000"))
		return mozart_x1000_linein_is_in();
	else if (!strcasecmp(buf, "m150"))
		return mozart_m150_linein_is_in();

	printf("Unknow cpu model, get linein state failed.\n");
	return false;
}

int mozart_linein_start(bool in_lock)
{
	if (linein_module.start) {
		return linein_module.start(&linein_module, module_cmd_stop, in_lock);
	} else {
		pr_err("linein_module isn't registered!\n");
		return -1;
	}
}

int mozart_linein_startup(void)
{
	if (mozart_module_register(&linein_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	return 0;
}

int mozart_linein_shutdown(void)
{
	if (linein_module.stop)
		linein_module.stop(&linein_module, module_cmd_stop, false);
	mozart_module_unregister(&linein_module);

	return 0;
}
