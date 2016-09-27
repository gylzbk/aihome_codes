#ifndef __MOZART_NET_H__
#define __MOZART_NET_H__

#define mozart_module_net_lock()			\
	do { \
		printf("+++++++++++++++++++++++++++++++++++++++++++net_lock\n");\
		printf("--- net lock----file: %s, func: %s, line: %d \n", __FILE__, __func__, __LINE__);\
		module_mutex_lock(&net_mode_lock);	\
	} while (0)

#define mozart_module_net_unlock()			\
	do {						\
		module_mutex_unlock(&net_mode_lock);	\
		printf("--- net unlock----file: %s, func: %s, line: %d \n", __FILE__, __func__, __LINE__);\
		printf("-------------------------------------------net_unlock\n");\
	} while (0)


extern bool __mozart_net_is_start(void);
extern int mozart_net_try_sta(void);
extern int create_wifi_config_pthread(void);
extern int mozart_net_startup(void);
extern void mozart_net_shutdown(void);

#endif /* __MOZART_NET_H__ */
