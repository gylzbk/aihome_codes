#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mozart_module.h"
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#ifndef MOZART_RELEASE
#define MOZART_KEY_DEBUG
#endif

#ifdef MOZART_KEY_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[KEY] %s: "fmt, __func__, ##args)
#else  /* MOZART_ATALK_ASR_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_ATALK_ASR_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[KEY] [Error] %s: "fmt, __func__, ##args)

//static int g_asr_flag;
//static int g_pressed;
static bool is_key_ignore  = false;
//pthread_mutex_t key_lock = PTHREAD_MUTEX_INITIALIZER;


#if 0
/*-----------------------------------------------------------------*/
static int middle_key_press_times = 0;
void _middle_key_check(int sig)
{
	g_asr_flag = 0;
	if (g_pressed){//&&(1==middle_key_press_times)) {
 		middle_key_press_times = 0;
 		g_asr_flag = 1;
		printf("voice toggle\n");
	//	mozart_prompt_tone_key_sync("atalk_wakeup_41", false);
	//remove_atalk			mozart_atalk_asr_wakeup();
	//	mozart_key_wakeup();
		return;
	}
	switch (middle_key_press_times) {
	case 1:
		printf("key press one times\n");
		mozart_module_next_song();
		break;
#if 0
	case 2:
		printf("boot_sel1 press two times\n");
		mozart_module_favorite();
		break;
#endif
	default:
		printf("key press two times\n");
		mozart_module_resume_pause();
		break;
	}
	middle_key_press_times = 0;
}

int middle_key_timer_start(int ms)
{
	int retval = 0;
	struct itimerval tick;

	signal(SIGALRM, _middle_key_check);
	memset(&tick, 0, sizeof(tick));

	/*Timeout to run first time*/
	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = ms;

	/*After first, the Interval time for clock*/
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
		printf("Set timer failed!\n");
		retval = -1;
	}

	return retval;
}

void middle_key_pressed(void)
{
	g_pressed = 1;
	middle_key_press_times++;
	if (middle_key_press_times == 1) {
		middle_key_timer_start(500000);
	}
	else {
		if (middle_key_press_times > 5) {
			middle_key_press_times = 0;
		}
	}		//*/
//	printf("\n\nmiddle_key_press_times = %d\n\n",middle_key_press_times);
}


void middle_key_released(void)
{
	g_pressed = 0;
	if (g_asr_flag == 1){
#if  SUPPORT_VR_SPEECH
		g_asr_flag = 0;
#else
		usleep(1000000);
		g_asr_flag = 0;
		middle_key_press_times =0;
		mozart_module_asr_cancel();
#endif
	}
}
#endif

bool mozart_key_ignore_get(void){
	return is_key_ignore;
}


int mozart_key_ignore_set(bool ignore){
	is_key_ignore = ignore;
	return 0;
}

/*-----------------------------------------------------------------*/
