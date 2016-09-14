#ifndef __MOZART_KEY_H__
#define __MOZART_KEY_H__

#if 0
extern pthread_mutex_t key_lock;

#define mozart_key_mutex_lock()			\
	do {						\
		module_mutex_lock(&key_lock);	\
	} while (0)

#define mozart_key_mutex_unlock()			\
	do {						\
		module_mutex_unlock(&key_lock);	\
	} while (0)



extern void middle_key_pressed(void);
extern void middle_key_released(void);
#endif

extern bool mozart_key_ignore_get(void);
extern int mozart_key_ignore_set(bool ignore);

#endif /* __MOZART_CHANNAL_H__ */

