#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "utils_interface.h"
#include "player_interface.h"
#include "volume_interface.h"
#include "render_interface.h"
#include "tips_interface.h"
#include "sharememory_interface.h"

#include "mozart_module.h"
#include "mozart_atalk.h"
#include "mozart_player.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"

#include "mozart_dmr.h"

#ifndef MOZART_RELEASE
#define MOZART_DMR_DEBUG
#else
#define MOZART_RELEASE_NAME
#endif

#ifdef MOZART_DMR_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[DMR] %s: "fmt, __func__, ##args)
#else  /* MOZART_DMR_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_DMR_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[DMR] [Error] %s: "fmt, __func__, ##args)

static bool dmr_first = true;
static struct player_context context;
static int dmr_resume_handler(void)
{
	int ret = 0;
	player_handler_t *handler = NULL;
	player_status_t status = PLAYER_UNKNOWN;

	pr_debug("...\n");
	if (context.uuid == NULL) {
		handler = mozart_player_handler_get("player_saver", NULL, NULL);
		if (!handler) {
			pr_err("player handler get\n");
			return -1;
		}

		status = mozart_player_getstatus(handler);
		if (status == PLAYER_PAUSED) {
			if (mozart_render_play_pause()) {
				pr_err("play_pause fail\n");
				ret = -1;
			}
		} else {
			pr_debug("player is %d\n", status);
		}

		mozart_player_handler_put(handler);
	} else {
		if (mozart_player_force_resume(context))
			pr_err("force_resume fail\n");

		free(context.uuid);
		free(context.url);
		context.uuid = NULL;
		context.url = NULL;
	}

	return ret;
}

static int dmr_pause_handler(void)
{
	player_handler_t *handler = NULL;
	player_status_t status = PLAYER_UNKNOWN;

	handler = mozart_player_handler_get("player_saver", NULL, NULL);
	if (!handler) {
		pr_err("player handler get\n");
		return -1;
	}

	memset(&context, 0, sizeof(struct player_context));
	status = mozart_player_getstatus(handler);
	if (status == PLAYER_PLAYING) {
		if (mozart_render_play_pause()) {
			pr_debug("play_pause fail\n");
			context = mozart_player_force_pause(handler);
		} else {
			if (mozart_player_wait_status(handler, PLAYER_PAUSED, 500 * 1000))
				context = mozart_player_force_pause(handler);
		}
	} else if (status == PLAYER_PAUSED) {
		pr_debug("player is paused\n");
	} else {
		pr_debug("player is %d\n", status);
		context = mozart_player_force_pause(handler);
	}

	mozart_player_handler_put(handler);

	return 0;
}

/*******************************************************************************
 * module
 *******************************************************************************/
static int dmr_module_start(struct mozart_module_struct *self)
{
	mozart_smartui_linein_play();
	if (self->module_change)
		__mozart_prompt_tone_key_sync("atalk_swich_false_5");
	self->player_state = player_state_idle;

	return 0;
}

static int dmr_module_run(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play)
		return dmr_resume_handler();
	else if (self->player_state == player_state_idle)
		self->player_state = player_state_play;

	return 0;
}

static int dmr_module_suspend(struct mozart_module_struct *self)
{
	if (self->player_state == player_state_play)
		return dmr_pause_handler();
	else
		return 0;
}

static int dmr_module_stop(struct mozart_module_struct *self)
{
	player_handler_t *handler = NULL;

	handler = mozart_player_handler_get("player_saver", NULL, NULL);
	if (!handler) {
		pr_err("player handler get\n");
		return -1;
	}

	mozart_render_stop_playback();
	if (mozart_player_wait_status(handler, PLAYER_STOPPED, 500 * 1000))
		mozart_player_force_stop(handler);

	mozart_player_handler_put(handler);
	self->player_state = player_state_idle;

	return 0;
}

static void dmr_module_volume_change(struct mozart_module_struct *self)
{
	int vol = mozart_volume_get();
	mozart_render_set_volume(vol);
}

static void dmr_module_next_module(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (__mozart_module_is_online())
		mozart_atalk_cloudplayer_start(true);
	else
		mozart_atalk_localplayer_start(true);

	mozart_module_mutex_unlock();
}

static void dmr_module_resume_pause(struct mozart_module_struct *self)
{
	mozart_module_mutex_lock();

	if (self->player_state == player_state_play) {
		/* pause */
		if (__mozart_module_is_run(self)) {
			dmr_pause_handler();
			self->player_state = player_state_pause;
		} else if (__mozart_module_is_start(self)) {
			self->player_state = player_state_pause;
		}
	} else if (self->player_state == player_state_pause) {
		/* resume */
		if (__mozart_module_is_run(self)) {
			dmr_resume_handler();
			self->player_state = player_state_play;
		} else if (__mozart_module_is_start(self)) {
			self->player_state = player_state_play;
		}
	}

	mozart_module_mutex_unlock();
}

static struct mozart_module_struct dmr_module = {
	.name = "dmr",
	.priority = 1,
	.attach = module_unattach,
	.mops = {
		.on_start   = dmr_module_start,
		.on_run     = dmr_module_run,
		.on_suspend = dmr_module_suspend,
		.on_stop    = dmr_module_stop,
	},
	.kops = {
		.volume_change = dmr_module_volume_change,
		.resume_pause = dmr_module_resume_pause,
		.next_module = dmr_module_next_module,
	},
};

/*******************************************************************************
 * API
 *******************************************************************************/
bool __mozart_dmr_is_start(void)
{
	return __mozart_module_is_start(&dmr_module);
}

int mozart_dmr_start(bool in_lock)
{
	module_status domain_status;
	int i;
	int err = 0;

	/* wait 0.5s */
	for (i = 0; i < 10; i++) {
		if (share_mem_get(RENDER_DOMAIN, &domain_status)) {
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

	if (dmr_module.start) {
		err = dmr_module.start(&dmr_module, module_cmd_stop, in_lock);
	} else {
		pr_err("dmr_module isn't registered!\n");
		err = -1;
	}

	share_mem_set(RENDER_DOMAIN, RESPONSE_DONE);

	return err;
}

int mozart_dmr_do_resume(void)
{
	int ret;
	struct mozart_module_struct *self = &dmr_module;

	mozart_module_mutex_lock();
	if (__mozart_module_is_run(self)) {
		ret = 1;
		self->player_state = player_state_play;
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
		self->player_state = player_state_play;
	} else {
		ret = -1;
	}
	mozart_module_mutex_unlock();

	pr_debug("ret = %d\n", ret);

	return ret;
}

int mozart_dmr_do_pause(void)
{
	int ret;
	struct mozart_module_struct *self = &dmr_module;

	mozart_module_mutex_lock();
	if (__mozart_module_is_run(self)) {
		self->player_state = player_state_pause;
		ret = 1;
	} else if (__mozart_module_is_start(self)) {
		ret = 0;
	} else {
		ret = -1;
	}
	mozart_module_mutex_unlock();

	pr_debug("ret = %d\n", ret);

	return ret;
}

static void AVTransportAction_callback(char *ActionName, struct Upnp_Action_Request *ca_event)
{
    printf("Not support AVTransportAction: %s\n", ActionName);
    return;
}

static void ConnectionManagerAction_callback(char *ActionName, struct Upnp_Action_Request *ca_event)
{
    printf("Not support ConnectionManagerAction: %s\n", ActionName);
    return;
}

static void RenderingControlAction_callback(char *ActionName, struct Upnp_Action_Request *ca_event)
{
    printf("Not support RenderingControlAction: %s\n", ActionName);
    return;
}

static void dmr_startup(void)
{
        int ret = 0;
        render_device_info_t device;

        memset(&device, 0, sizeof(device));
        /*
         * this is our default frendlyname rule in librender.so,
         * and you can create your own frendlyname rule.
         */
        char deviceName[64] = {};
        char macaddr[] = "255.255.255.255";
        memset(macaddr, 0, sizeof (macaddr));

#ifdef MOZART_RELEASE_NAME
	sprintf(deviceName, "DS-1825");
#else
        //FIXME: replace "wlan0" with other way. such as config file.
        get_mac_addr("wlan0", macaddr, "");
        sprintf(deviceName, "SmartAudio-%s", macaddr + 4);
#endif
        device.friendlyName = deviceName;
	printf("name: %s\n", device.friendlyName);

        if (mozart_render_action_callback(AVTransportAction_callback,
                                             ConnectionManagerAction_callback,
                                             RenderingControlAction_callback))
                printf("[warning] register render action callback failed.\n");

        ret = mozart_render_start(&device);
        if (ret)
                printf("[Error] start render service failed.\n");

	dmr_first = false;
}

int mozart_dmr_startup(void)
{
	if (mozart_module_register(&dmr_module)) {
		pr_err("mozart_module_register fail\n");
		return -1;
	}

	if (dmr_first)
		dmr_startup();
	else
		mozart_render_refresh();

	return 0;
}

int mozart_dmr_shutdown(void)
{
	if (dmr_module.stop && __mozart_dmr_is_start())
		dmr_module.stop(&dmr_module, module_cmd_stop, false);
	if (!mozart_module_unregister(&dmr_module))
		mozart_render_terminate_fast();

	return 0;
}
