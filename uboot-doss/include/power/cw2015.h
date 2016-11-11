/*

*/
#ifndef  _CW2015_H_
#define _CW2015_H_

#define REG_VERSION             0x0
#define REG_VCELL               0x2
#define REG_SOC                 0x4
#define REG_RRT_ALERT           0x6
#define REG_CONFIG              0x8
#define REG_MODE                0xA
#define REG_BATINFO             0x10

#define MODE_SLEEP_MASK         (0x3<<6)
#define MODE_SLEEP              (0x3<<6)
#define MODE_NORMAL             (0x0<<6)
#define MODE_QUICK_START        (0x3<<4)
#define MODE_RESTART            (0xf<<0)

#define CONFIG_UPDATE_FLG       (0x1<<1)
#define ATHD                    (0xa<<3)        //ATHD = 10%

//#define x_debug

#ifdef x_debug
#define debug_x(fmt, args...) \
	printf("[cw2015] --> "fmt, ##args)
#else
#define debug_x(fmt, args...) \
	do{}while(0)
#endif

#endif
