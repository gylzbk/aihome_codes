#ifndef __MOZART_APP_H__
#define __MOZART_APP_H__

#define APP_DEPEND_NET		(1 << 0)
#define APP_DEPEND_NET_AP	(1 << 1)
#define APP_DEPEND_NET_STA	(1 << 2)
#define APP_DEPEND_NET_ALL	(APP_DEPEND_NET | APP_DEPEND_NET_AP | APP_DEPEND_NET_STA)

#define APP_DEPEND_NO_NET	(1 << 3)
#define APP_DEPEND_ALL		(APP_DEPEND_NET_ALL | APP_DEPEND_NO_NET)

extern int startall(int depend_network);
extern int stopall(int depend_network);

#endif	/* __MOZART_APP_H__ */
