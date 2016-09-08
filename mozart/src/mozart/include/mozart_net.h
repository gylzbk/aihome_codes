#ifndef __MOZART_NET_H__
#define __MOZART_NET_H__

extern bool __mozart_net_is_start(void);
extern int mozart_net_try_sta(void);
extern int create_wifi_config_pthread(void);
extern int mozart_net_startup(void);
extern void mozart_net_shutdown(void);

#endif /* __MOZART_NET_H__ */
