#ifndef __AI_SOUND_DEV_H__
#define __AI_SOUND_DEV_H__

#include "aiengine_app.h"

#define AEC_SIZE 1024
//#define RECORD_BUF_SZ		1024
//#define OPT_BUF_SZ		1024
#define CHANEL_1		1
#define CHANEL_4		4
#define RATE			16000
#define BIT				16
#define VOLUME				50
#define VOLUME_AEC_HIGHT			40	//	35	//	20
#define VOLUME_AEC_LOUDLY			40	//	40	//	30
#define VOLUME_AEC_LOW				40	//	45	//	35
#define CHANEL	1
#define MS		32
/* ioctl command */
#define SNDCTL_DSP_AEC_START _SIOW ('J', 16, int)
#define SNDCTL_DSP_AEC_ABLE  _SIOW ('J', 17, int)

//#define AEC_FILE_DEBUG

/* function declaration */
int pipe_init(void);
void pipe_close(void);
int sound_device_init_near(int val);
int sound_device_init_far(void);
int sound_device_init(int vol);
int sound_aec_enable(void);
int sound_aec_disable(void);
void *dmic_read(void *args);
void *loopback_read(void *args);
void *aec_handle(void *args);
void sound_device_release(void);
extern int aec_wakeup_flag;
#endif

