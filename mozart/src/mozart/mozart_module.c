#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#include "power_interface.h"
#include "utils_interface.h"
#include "volume_interface.h"

#include "mozart_module.h"
#include "mozart_net.h"
#include "mozart_smartui.h"
#include "mozart_prompt_tone.h"
#include "mozart_update_control.h"
#include "baselib.h"

#include "mozart_config.h"
#if (SUPPORT_VR == VR_ATALK)
#include "mozart_atalk.h"
#elif (SUPPORT_VR == VR_SPEECH)
#include "mozart_aitalk.h"
#endif

#ifndef MOZART_RELEASE
#define MOZART_MODULE_DEBUG
#endif

#ifdef MOZART_MODULE_DEBUG
#define pr_debug(fmt, args...)				\
	printf("[MODULE] %-30s: "fmt, __func__, ##args)
#else  /* MOZART_MODULE_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif	/* MOZART_MODULE_DEBUG */

#define pr_err(fmt, args...)						\
	fprintf(stderr, "[MODULE] [Error] %-30s: "fmt, __func__, ##args)

static int __mozart_module_stop(struct mozart_module_struct *self,
				      enum mozart_module_command command);
/*******************************************************************************
 * module
 *******************************************************************************/
static int invalid_module_start(struct mozart_module_struct *current)
{
	return 0;
}

static int invalid_module_run(struct mozart_module_struct *current)
{
	return 0;
}

static int invalid_module_suspend(struct mozart_module_struct *current)
{
	return 0;
}

static int invalid_module_stop(struct mozart_module_struct *current)
{
	return 0;
}

static struct mozart_module_struct invalid_module = {
	.name = "invalid",
	.priority = -1,
	.state = module_state_run,
	.attach = module_attach_do_not_care,
	.mops = {
		.on_start   = invalid_module_start,
		.on_run     = invalid_module_run,
		.on_suspend = invalid_module_suspend,
		.on_stop    = invalid_module_stop,
	},
};

/*******************************************************************************
 * State API
 *******************************************************************************/
enum mozart_module_state_bit {
	MODULE_STATE_NET_BIT = 0,
	MODULE_STATE_ATTACH_BIT,
	MODULE_STATE_ONLINE_BIT,
};

pthread_mutex_t module_lock = PTHREAD_MUTEX_INITIALIZER;

static int module_nr = 1;
static struct mozart_module_struct *current_module = &invalid_module;
static int global_state_map = 1 << MODULE_STATE_ATTACH_BIT | 1 << MODULE_STATE_NET_BIT;

bool __mozart_module_is_net(void)
{
	return global_state_map & (1 << MODULE_STATE_NET_BIT);
}

bool __mozart_module_is_attach(void)
{
	return global_state_map & (1 << MODULE_STATE_ATTACH_BIT);
}

bool __mozart_module_is_online(void)
{
	return global_state_map & (1 << MODULE_STATE_ONLINE_BIT);
}

void __mozart_module_set_net(void)
{
	global_state_map |= (1 << MODULE_STATE_NET_BIT);
}

void __mozart_module_clear_net(void)
{
	global_state_map &= ~(1 << MODULE_STATE_NET_BIT);
}

void __mozart_module_set_attach(void)
{
	global_state_map |= (1 << MODULE_STATE_ATTACH_BIT);
}

void __mozart_module_set_unattach(void)
{
	global_state_map &= ~(1 << MODULE_STATE_ATTACH_BIT);
}

void __mozart_module_set_online(void)
{
	global_state_map |= (1 << MODULE_STATE_ONLINE_BIT);
	mozart_smartui_update_hide(false);
	#if (SUPPORT_VR == VR_ATALK)
	if (!__mozart_module_is_attach())
		mozart_smartui_wifi_update();
	#elif (SUPPORT_VR == VR_SPEECH)
		mozart_smartui_wifi_update();
	#endif

}

void __mozart_module_set_offline(void)
{
	global_state_map &= ~(1 << MODULE_STATE_ONLINE_BIT);
	mozart_smartui_update_hide(true);
	mozart_smartui_wifi_update();
}

void __mozart_module_dump_state_map(void)
{
	pr_debug("================================\n");
	pr_debug("Global Net State: %s(%s)\n",
		 __mozart_module_is_online() ? "Online" : "Offline",
		 __mozart_module_is_net() ? "Config" : "Normal");
	pr_debug("Global Attach State: %s\n",
		 __mozart_module_is_attach() ? "Attach" : "Unattach");
	pr_debug("================================\n");
}

static void mozart_common_volume_up(void)
{
	int vol;

	vol = mozart_volume_get();
	if (vol == 100)
		return ;

	vol += 10;
	if (vol > 100)
		vol = 100;
	else if (vol < 0)
		vol = 0;
	vol = vol / 10 * 10;

	mozart_volume_set(vol, MUSIC_VOLUME);
}

static void mozart_common_volume_down(void)
{
	int vol;

	vol = mozart_volume_get();
	if (vol == 0)
		return ;

	vol -= 10;
	if (vol > 100)
		vol = 100;
	else if (vol < 0)
		vol = 0;
	vol = vol / 10 * 10;

	mozart_volume_set(vol, MUSIC_VOLUME);
}

static struct mozart_module_struct *mozart_module_get(void)
{
	struct mozart_module_struct *module = current_module;

	while (module->flags & MODULE_FLAG_CHILD && module->parent)
		module = module->parent;
	return module;
}

void mozart_module_is_playing(void)
{
	struct mozart_module_struct *module;
	bool (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.is_playing;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

static int mozart_module_current_stop(void)
{
	//mozart_module_mutex_lock();

	__mozart_module_stop(current_module, module_cmd_stop);

	/* mozart_module_mutex_unlock(); */

	return 0;
}

void mozart_module_previous_song(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.previous_song;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_next_song(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.next_song;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_volume_change(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = current_module;
	func = module->kops.volume_change;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_volume_up(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = current_module;
	func = module->kops.volume_up;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	} else {
		pr_debug("common\n");
		mozart_common_volume_up();
	}
}

void mozart_module_volume_down(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = current_module;
	func = module->kops.volume_down;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	} else {
		pr_debug("common\n");
		mozart_common_volume_down();
	}
}

void mozart_module_resume_pause(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.resume_pause;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_wifi_config(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.wifi_config;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	} else {
		pr_debug("common\n");
		create_wifi_config_pthread();
	}
}

void mozart_module_asr_wakeup(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.asr_wakeup;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_asr_cancel(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.asr_cancel;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_next_channel(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.next_channel;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_favorite(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.favorite;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_factory_reset(void)
{
	mozart_module_current_stop();
	mozart_smartui_boot_factory_reset();
	__mozart_prompt_tone_key_sync("atalk_factory_reset_36");
	mozart_smartui_shutdown();

#define RESET_TEST

#ifdef RESET_TEST
	mozart_system("rm -rf /mnt/sdcard/music/*");
	mozart_system("rm -rf /usr/data/music_list.json");	//--- clean music list
	mozart_system("rm -rf /usr/data/bsa");
	mozart_system("rm -rf /usr/data/network_manager.ini");
	mozart_system("rm -rf /usr/data/render.ini");
	mozart_system("rm -rf /usr/data/system.ini");
	mozart_system("rm -rf /usr/data/wpa_supplicant.conf");
	mozart_system("cp -a /usr/share/data/* /usr/data");
	system("reboot");
#else
	mozart_system("rm -rf /usr/data/music_list.json");	//--- clean music list
	printf("%s. Reset config file and empty sd card\n", __func__);
	/* Reset config file */
	mozart_system("rm -rf /mnt/sdcard/music/*");
	mozart_system("rm -rf /usr/data/*");
	mozart_system("cp -a /usr/share/data/* /usr/data");
	system("reboot");
#endif
}

void mozart_module_next_module(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.next_module;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_disconnect_handler(void)
{
	struct mozart_module_struct *module;
	void (*func)(struct mozart_module_struct *self);

	mozart_module_mutex_lock();
	module = mozart_module_get();
	func = module->kops.disconnect_handler;
	mozart_module_mutex_unlock();

	if (func) {
		pr_debug("%s\n", module->name);
		func(module);
	}
}

void mozart_module_power_off(char *reason)
{
	mozart_module_current_stop();
	mozart_smartui_boot_power_off(reason);
	__mozart_prompt_tone_key_sync("power_off");
	mozart_smartui_shutdown();
	//mozart_power_off();
	system("poweroff");
}

/*******************************************************************************
 * Module API
 *******************************************************************************/
static char *state_str[] = {
	[module_state_start]   = "module_state_start",
	[module_state_run]     = "module_state_run",
	[module_state_suspend] = "module_state_suspend",
	[module_state_stop]    = "module_state_stop",
};

static char *cmd_str[] = {
	[module_cmd_invalid] = "module_cmd_invalid",
	[module_cmd_run]     = "module_cmd_run",
	[module_cmd_suspend] = "module_cmd_suspend",
	[module_cmd_stop]    = "module_cmd_stop",
};

#define mozart_module_dump_start(self, command, src, dst)		\
	do {								\
		pr_debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"); \
		pr_debug("++++++++ %s's %s action\n", self->name,	\
			 cmd_str[command]);				\
		pr_debug("++++++++ current_module: %s(%s)\n",		\
			 current_module->name,				\
			 state_str[current_module->state]);		\
		pr_debug("======== src: %s(%s)\n",			\
			 src->name, state_str[src->state]);		\
		pr_debug("======== src->parent: %s(%s) child: %s(%s)\n", \
			 src->parent ? src->parent->name : "NULL",	\
			 src->parent ?					\
			 state_str[src->parent->state] : "NULL",	\
			 src->child ? src->child->name : "NULL",	\
			 src->child ?					\
			 state_str[src->child->state] : "NULL");	\
		pr_debug("======== dst: %s(%s)\n",			\
			 dst->name, state_str[dst->state]);		\
		pr_debug("======== dst->parent: %s(%s) child: %s(%s)\n", \
			 dst->parent ? dst->parent->name : "NULL",	\
			 dst->parent ?					\
			 state_str[dst->parent->state] : "NULL",	\
			 dst->child ? dst->child->name : "NULL",	\
			 dst->child ?					\
			 state_str[dst->child->state] : "NULL");	\
		pr_debug("Global Net State: %s(%s)\n",			\
			 __mozart_module_is_online() ? "Online" : "Offline", \
			 __mozart_module_is_net() ? "Config" : "Normal"); \
		pr_debug("Global Attach State: %s\n",			\
			 __mozart_module_is_attach() ? "Attach" : "Unattach"); \
	} while (0)

#define mozart_module_dump_debug_over()					\
	do {								\
		pr_debug("-------- current: %s(%s)\n",			\
			 current_module->name,				\
			 state_str[current_module->state]);		\
		pr_debug("         current->parent: %s(%s) child: %s(%s)\n", \
			 current_module->parent ?			\
			 current_module->parent->name : "NULL",		\
			 current_module->parent ?			\
			 state_str[current_module->parent->state] : "NULL", \
			 current_module->child ?			\
			 current_module->child->name : "NULL",		\
			 current_module->child ?			\
			 state_str[current_module->child->state] : "NULL"); \
		pr_debug("------------------------------------------------------------------\n"); \
	} while (0)

#define mozart_module_dump_err_over()					\
	do {								\
		pr_err("-------- current: %s(%s)\n",			\
		       current_module->name,				\
		       state_str[current_module->state]);		\
		pr_err("         current->parent: %s(%s) child: %s(%s)\n", \
		       current_module->parent ?				\
		       current_module->parent->name : "NULL",		\
		       current_module->parent ?				\
		       state_str[current_module->parent->state] : "NULL", \
		       current_module->child ?				\
		       current_module->child->name : "NULL",		\
		       current_module->child ?				\
		       state_str[current_module->child->state] : "NULL"); \
		pr_err("------------------------------------------------------------------\n"); \
	} while (0)

static int __mozart_module_run_one(struct mozart_module_struct *module)
{

	int ret = -1;

	if ((module->state == module_state_start || module->state == module_state_suspend) &&
	    module->mops.on_run && module->mops.on_run(module) == 0) {
		module->state = module_state_run;
		ret = 0;
	}
	pr_debug("    %s->mops.on_run, ret = %d\n", module->name, ret);

	return ret;
}

static int __mozart_module_suspend_one(struct mozart_module_struct *module)
{

	int ret = -1;

	if ((module->state == module_state_start || module->state == module_state_run) &&
	    module->mops.on_suspend && module->mops.on_suspend(module) == 0) {
		module->state = module_state_suspend;
		ret = 0;
	}
	pr_debug("    %s->mops.on_suspend, ret = %d\n", module->name, ret);

	return ret;
}

static int __mozart_module_stop_one(struct mozart_module_struct *module)
{
	int ret = -1;

	if (module->mops.on_stop && module->mops.on_stop(module) == 0) {
		module->state = module_state_stop;
		if (module->child) {
			if (module->parent == NULL)
				module->child->parent = &invalid_module;
			else
				module->child->parent = module->parent;
		}
		if (module->parent)
			module->parent->child = module->child;

		if (current_module == module) {
			if (module->parent == NULL)
				current_module = &invalid_module;
			else
				current_module = module->parent;
			current_module->state = module_state_start;
		}

		module->child = module->parent = NULL;
		ret = 0;
	}
	pr_debug("    %s->mops.on_stop, ret = %d\n", module->name, ret);

	return ret;
}

static int __mozart_module_start_run_one(struct mozart_module_struct *module)
{

	int ret = -1;

	if (module->mops.on_start && module->mops.on_start(module) == 0) {
		module->state = module_state_start;
		current_module = module;
		ret = 0;
	}
	pr_debug("    %s->mops.on_start, ret = %d\n", module->name, ret);
	if (ret)
		return ret;

	if (module->mops.on_run && module->mops.on_run(module) == 0) {
		module->state = module_state_run;
		ret = 0;
	} else {
		ret = -1;
	}
	pr_debug("    %s->mops.on_run, ret = %d\n", module->name, ret);
	if (ret)
		__mozart_module_stop_one(module);

	return ret;
}

static int __mozart_module_stop_all(struct mozart_module_struct *m)
{
	struct mozart_module_struct *parent;
	struct mozart_module_struct *module = current_module;

	while (module) {
		parent = module->parent;
		if (__mozart_module_stop_one(module))
			return -1;
		if (module == m)
			return 0;
		module = parent;
	}

	return 0;
}

static int __mozart_module_compare_priority_all(struct mozart_module_struct *m,
						struct mozart_module_struct *dst)
{
	struct mozart_module_struct *src = current_module;

	while (src) {
		pr_debug("src: %s->priority = %d, dst: %s->priority = %d\n",
			 src->name, src->priority, dst->name, dst->priority);

		if (!(src->flags & MODULE_FLAG_CHILD) && src->priority > dst->priority)
			return -1;
		if (src == m)
			return 0;
		src = src->parent;
	}

	return 0;
}

static inline
struct mozart_module_struct *__mozart_module_get_parent(struct mozart_module_struct *module)
{
	while (module && module->parent)
		module = module->parent;

	return module;
}

bool __mozart_module_is_start(struct mozart_module_struct *module)
{
	return module->state != module_state_stop;
}

bool __mozart_module_is_run(struct mozart_module_struct *module)
{
	return module->state == module_state_run ||
		(module->child && module->child->flags & MODULE_FLAG_CHILD &&
		 module->child->state == module_state_run);
}

static int __mozart_module_start(struct mozart_module_struct *self,
				 enum mozart_module_command command)
{
	int ret;
	struct mozart_module_struct *src, *dst;

	if (self == NULL) {
		pr_err("self is NULL\n");
		return -1;
	}

	if (command != module_cmd_suspend && command != module_cmd_stop) {
		pr_err("command is %s\n", cmd_str[command]);
		return -1;
	}

	if (__mozart_module_is_start(self)) {
		pr_debug("%s is %s\n", self->name, state_str[self->state]);
		return 0;
	}

	if (command == module_cmd_stop)
		src = __mozart_module_get_parent(current_module);
	else
		src = current_module;
	dst = self;
	dst->module_change = false;

	mozart_module_dump_start(self, command, src, dst);

	if (command == module_cmd_stop) {
		ret = __mozart_module_compare_priority_all(src, dst);
		pr_debug("    call %s's compare_priority_all, ret = %d\n", src->name, ret);
		if (ret)
			goto err;

		ret = __mozart_module_stop_all(src);
		pr_debug("    call %s's stop_all, ret = %d\n", src->name, ret);
		if (ret)
			goto err;

		dst->module_change = true;
		if (__mozart_module_start_run_one(dst))
			goto err;

		if (dst->attach == module_attach && !__mozart_module_is_attach()) {
			pr_debug("%s -> %s, switch_mode true\n", src->name, dst->name);
			#if (SUPPORT_VR == VR_ATALK)
				ret = __mozart_atalk_switch_mode(true);
			#elif (SUPPORT_VR == VR_SPEECH)
				ret = __mozart_aitalk_switch_mode(true);
			#endif

		} else if (dst->attach == module_unattach && __mozart_module_is_attach()) {
			pr_debug("%s -> %s, switch_mode false\n", src->name, dst->name);
			#if (SUPPORT_VR == VR_ATALK)
				ret = __mozart_atalk_switch_mode(false);
			#elif (SUPPORT_VR == VR_SPEECH)
				ret = __mozart_aitalk_switch_mode(false);
			#endif
		}
		pr_debug("    switch_mode ret = %d\n", ret);
		if (ret) {
			__mozart_module_stop_one(dst);
			goto err;
		}

	} else if (command == module_cmd_suspend) {
		pr_debug("src: %s->priority = %d, dst: %s->priority = %d\n",
			 src->name, src->priority, dst->name, dst->priority);
		if (src->priority > dst->priority)
			goto err;

		if ((src->flags & MODULE_FLAG_CHILD) && src->parent) {
			if (__mozart_module_stop_one(src))
				goto err;
			src = current_module;
		} else {
			if (__mozart_module_suspend_one(src))
				goto err;
		}

		if (__mozart_module_start_run_one(dst)) {
			__mozart_module_run_one(current_module);
			goto err;
		}

		src->child = dst;
		dst->parent = src;
	}

	mozart_module_dump_debug_over();
	return 0;

err:
	mozart_module_dump_err_over();

	return -1;
}

static int mozart_module_start(struct mozart_module_struct *self,
			       enum mozart_module_command command, bool in_lock)
{
	int ret = 0;

	if (!in_lock)
		mozart_module_mutex_lock();

	ret = __mozart_module_start(self, command);

	if (!in_lock)
		mozart_module_mutex_unlock();

	return ret;
}

static int __mozart_module_stop(struct mozart_module_struct *self,
				enum mozart_module_command command)
{
	int ret;
	struct mozart_module_struct *src, *dst;

	if (self == NULL) {
		pr_err("self is NULL\n");
		return -1;
	}

	if (command != module_cmd_run && command != module_cmd_stop) {
		pr_err("command is %s\n", cmd_str[command]);
		return -1;
	}

	if (!__mozart_module_is_start(self)) {
		pr_debug("%s is %s\n", self->name, state_str[self->state]);
		return 0;
	}

	if (command == module_cmd_stop) {
		pr_debug("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		pr_debug("++++++++ %s's %s action\n", self->name, cmd_str[command]);
		pr_debug("++++++++ current: %s(%s)\n", current_module->name, state_str[current_module->state]);

		ret = __mozart_module_stop_all(self);
		pr_debug("    call %s's stop_all, ret = %d\n", self->name, ret);
		if (ret)
			goto err;

	} else if (command == module_cmd_run) {
		src = self;
		dst = self->parent;

		if (dst == NULL) {
			pr_err("dst is NULL\n");
			goto err;
		}

		mozart_module_dump_start(self, command, src, dst);

		if (__mozart_module_is_run(src)) {
			ret = __mozart_module_stop_all(src);
			pr_debug("    call %s's stop_all, ret = %d\n", src->name, ret);
			if (ret)
				goto err;

			if (__mozart_module_run_one(dst))
				goto err;

		} else if (__mozart_module_is_start(src)) {
			if (__mozart_module_stop_one(src))
				goto err;
		}
	}

	mozart_module_dump_debug_over();
	return 0;

err:
	mozart_module_dump_err_over();
	return -1;
}

static int mozart_module_stop(struct mozart_module_struct *self,
			      enum mozart_module_command command, bool in_lock)
{
	int ret = 0;

	if (!in_lock)
		mozart_module_mutex_lock();

	ret = __mozart_module_stop(self, command);

	if (!in_lock)
		mozart_module_mutex_unlock();

	return ret;
}

/*******************************************************************************
 * Register
 *******************************************************************************/
int mozart_module_register(struct mozart_module_struct *module)
{
	if (module == NULL) {
		pr_err("module is NULL!\n");
		return -1;
	}

	if (module->name == NULL) {
		pr_err("name is NULL!\n");
		return -1;
	}

	pr_debug("%s register\n", module->name);

	if (module->attach < module_attach_do_not_care ||
	    module->attach > module_unattach) {
		pr_err("attach is invalid!\n");
		return -1;
	}

	if ((module->start != NULL) || (module->stop != NULL))
		return 0;

	module->uuid = module_nr++;
	module->start = mozart_module_start;
	module->stop = mozart_module_stop;

	return 0;
}

int mozart_module_unregister(struct mozart_module_struct *module)
{
	if (module == NULL) {
		pr_err("module is NULL!\n");
		return -1;
	}

	if (module->name == NULL) {
		pr_err("name is NULL!\n");
		return -1;
	}

	pr_debug("%s unregister\n", module->name);

	if ((module->start == NULL) || (module->stop == NULL))
		return -1;

	module->uuid = 0;
	module->start = NULL;
	module->stop = NULL;

	return 0;
}
