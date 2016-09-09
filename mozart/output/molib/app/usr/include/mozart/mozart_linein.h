#ifndef __MOZART_LINEIN_H__
#define __MOZART_LINEIN_H__

#include <stdbool.h>

extern bool __mozart_linein_is_start(void);
extern bool mozart_linein_is_in(void);
extern int mozart_linein_start(bool in_lock);
extern int mozart_linein_startup(void);
extern int mozart_linein_shutdown(void);

#endif /* __LINEIN_H__ */
