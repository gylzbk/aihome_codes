#ifndef __MOZART_BATTERY_H__
#define __MOZART_BATTERY_H__

#include "mozart_config.h"

#if (SUPPORT_BOARD == BOARD_PUBLIC)
extern int inline mozart_battery_update(void)
{
	return 0;
}
extern int  inline mozart_battery_startup(void)
{
	return 0;
}
extern int  inline mozart_battery_shutdown(void)
{
	return 0;
}
#else
extern int mozart_battery_update(void);
extern int mozart_battery_startup(void);
extern int mozart_battery_shutdown(void);
#endif

#endif /* __MOZART_BATTERY_H__ */
