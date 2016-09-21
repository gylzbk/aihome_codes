#ifndef __MOZART_MODULE_H__
#define __MOZART_MODULE_H__

#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
struct mozart_module_struct;

enum module_attach_enum {
	module_attach_invalid = 0,
	module_attach_do_not_care,
	module_attach,
	module_unattach,
};

enum mozart_module_command {
	module_cmd_invalid = 0,
	module_cmd_run,
	module_cmd_suspend,
	module_cmd_stop,
};

enum mozart_module_state_enum {
	module_state_stop = 0,
	module_state_suspend,
	module_state_run,
	module_state_start,
};

enum mozart_player_state_enum {
	player_state_idle = 0,
	player_state_play,
	player_state_pause,
};

struct mozart_module_ops {
	int (*on_start)(struct mozart_module_struct *self);
	int (*on_run)(struct mozart_module_struct *self);
	int (*on_suspend)(struct mozart_module_struct *self);
	int (*on_stop)(struct mozart_module_struct *self);
};

struct mozart_module_key_ops {
	void (*previous_song)(struct mozart_module_struct *self);
	void (*next_song)(struct mozart_module_struct *self);
	void (*volume_change)(struct mozart_module_struct *self);
	void (*volume_up)(struct mozart_module_struct *self);
	void (*volume_down)(struct mozart_module_struct *self);
	void (*resume_pause)(struct mozart_module_struct *self);
	void (*wifi_config)(struct mozart_module_struct *self);
	void (*asr_wakeup)(struct mozart_module_struct *self);
	void (*asr_cancel)(struct mozart_module_struct *self);
	void (*next_channel)(struct mozart_module_struct *self);
	void (*next_module)(struct mozart_module_struct *self);
	void (*favorite)(struct mozart_module_struct *self);
	void (*disconnect_handler)(struct mozart_module_struct *self);
	bool (*is_playing)(struct mozart_module_struct *self);
};

struct mozart_module_struct {
	bool module_change;
	char *name;
	int uuid;
	int priority;
	int flags;
#define MODULE_FLAG_CHILD (1 << 0)
	enum module_attach_enum attach;
	enum mozart_module_state_enum state;
	enum mozart_player_state_enum player_state;

	struct mozart_module_struct *parent;
	struct mozart_module_struct *child;

	struct mozart_module_ops mops;
	struct mozart_module_key_ops kops;

	int (*start)(struct mozart_module_struct *self, enum mozart_module_command command, bool in_lock);
	int (*stop)(struct mozart_module_struct *self, enum mozart_module_command command, bool in_lock);
};

#define module_mutex_lock(lock)						\
	do {								\
		int i = 0;						\
									\
		while (pthread_mutex_trylock(lock)) {			\
			if (i++ >= 100) {				\
				pr_err("#### {%s, %s, %d} dead lock####\n", \
				       __FILE__, __func__, __LINE__);	\
				i = 0;					\
			}						\
			usleep(100 * 1000);				\
		}							\
	} while (0)
#define module_mutex_unlock(lock) pthread_mutex_unlock(lock)

#define mozart_module_mutex_lock()			\
	do {						\
		printf("-----lock----file: %s, func: %s, line: %d \n", __FILE__, __func__, __LINE__);\
		module_mutex_lock(&module_lock);	\
	} while (0)

#define mozart_module_mutex_unlock()			\
	do {						\
		module_mutex_unlock(&module_lock);	\
		printf("-----unlock----file: %s, func: %s, line: %d \n", __FILE__, __func__, __LINE__);\
	} while (0)

extern char *global_app_name;
extern pthread_mutex_t module_lock;

extern bool __mozart_module_is_net(void);
extern bool __mozart_module_is_attach(void);
extern bool __mozart_module_is_online(void);
extern void __mozart_module_set_net(void);
extern void __mozart_module_clear_net(void);
extern void __mozart_module_set_attach(void);
extern void __mozart_module_set_unattach(void);
extern void __mozart_module_set_online(void);
extern void __mozart_module_set_offline(void);
extern void __mozart_module_dump_state_map(void);

extern void mozart_module_previous_song(void);
extern void mozart_module_next_song(void);
extern void mozart_module_volume_change(void);
extern void mozart_module_volume_up(void);
extern void mozart_module_volume_down(void);
extern void mozart_module_resume_pause(void);
extern void mozart_module_wifi_config(void);
extern void mozart_module_asr_wakeup(void);
extern void mozart_module_asr_cancel(void);
extern void mozart_module_next_channel(void);
extern void mozart_module_favorite(void);
extern void mozart_module_factory_reset(void);
extern void mozart_module_next_module(void);
extern void mozart_module_disconnect_handler(void);
extern void mozart_module_power_off(char *reason);

extern bool __mozart_module_is_start(struct mozart_module_struct *module);
extern bool __mozart_module_is_run(struct mozart_module_struct *module);

extern int mozart_module_register(struct mozart_module_struct *module);
extern int mozart_module_unregister(struct mozart_module_struct *module);

#endif /* __MOZART_MODULE_H__ */
