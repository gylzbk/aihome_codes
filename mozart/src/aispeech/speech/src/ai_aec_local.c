#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/soundcard.h>
#include "cJSON.h"
#include "echo_wakeup.h"
#include "ai_sound_dev.h"
#include "aiengine_app.h"

#define THREAD_ERROR_NO_RESTART 0

int error_flag = false;
int aec_wakeup_flag = AEC_IDEL;
int g_aec_stop_flag = 0;
static int ai_aec_working = 0;

int is_dmic_running = false;
int is_loopback_running = false;
int is_aec_read_running = false;

bool aec_end_flag = false;

extern int fddmic[2];
extern int fdplay[2];


static const char *param = "{"
"   \"request\": {"
"       \"env\": \"words=你好小乐;\""
"   }"
"}";

int _wakeup_aec_callback(const void *usrdata, const char *id,
                            int type,const void *message, int size)
{
    //printf("resp data: %.*s\n", size, (char *) message);
    cJSON *out = NULL;
	out = cJSON_Parse((char*) message);
    if (!out)
    {
        return -1;
    }

    cJSON *result = NULL;
	result = cJSON_GetObjectItem(out, "result");
    if (result)
    {
        cJSON *wakeupWrd = NULL;
		wakeupWrd = cJSON_GetObjectItem(result, "wakeupWord");
        if (wakeupWrd && !strcmp(wakeupWrd->valuestring, "你 好 小 乐"))
        {
            DEBUG("=======>唤醒成功<=======\n");
            aec_wakeup_flag = AEC_WAKEUP;
		//	aec_end_flag = true;
		//	ai_tone_time_start();
		//	system("killall -9 mplayer");//add by zhangliang for test
        }
    }

    if (out)
    {
        cJSON_Delete(out);
    }
    return 0;
}

int  ai_aec_stoping(void){
	int timeout = 0;
	while(g_aec_stop_flag){
		usleep(1000);		//	100ms*50 = 5s
		if (++timeout > 5000){
			break;
		}
	}
	return 0;
}

void  ai_aec_stop(void){
	DEBUG("=================> Stop aec!\n");
	if (ai_aec_working){
		aec_end_flag = true;
		if (aec_wakeup_flag == AEC_START){
			g_aec_stop_flag = 1;
			aec_wakeup_flag = AEC_WAKEUP;
			ai_aec_stoping();
		}
		aec_end_flag = false;
	}
}
#ifdef AEC_FILE_DEBUG
	int record_count;
	int fdr = -1;
	int fdp = -1;
#endif
/* AEC entry */
int ai_aec(echo_wakeup_t *ew)
{
    int ret = 0;
	char bufr[AEC_SIZE] = {0};
	char bufp[AEC_SIZE] = {0};
    pthread_t tid1 = 0;
    pthread_t tid2 = 0;
//    pthread_t tid3 = 0;
	ai_aec_working = 1;
    aec_end_flag = false;
	error_flag = false;
    aec_wakeup_flag = AEC_START;
	g_aec_stop_flag = 0;
	int aec_able_status = false;
	int volume_set = VOLUME_AEC;
	int status = 0;
/*	int vol = mozart_volume_get();
	if (vol >= 70){
		volume_set = VOLUME_AEC_HIGHT;
	}
	else if (vol >= 40){
		volume_set = VOLUME_AEC_LOUDLY;
	}
	else{
		volume_set = VOLUME_AEC_LOW;
	}	//*/
//	printf("\n==========11111111111====== vol = %d ,vol_aec = %d ==========111111111======\n",vol,volume_set);
//	record_param rparam = {BIT, RATE, CHANEL_1, volume_set};
#ifdef AEC_FILE_DEBUG
	struct   timeval    time_s;
	gettimeofday(&time_s,NULL);
	int vol =   mozart_volume_get();
	char name_file[100];
	memset(name_file,0,100);
	sprintf(name_file,"/mnt/sdcard/dmic_%6d_%d.pcm",time_s.tv_sec,vol);
	fdr = open(name_file,O_RDWR|O_CREAT|O_TRUNC,0777);
	memset(name_file,0,100);
	sprintf(name_file,"/mnt/sdcard/loopback_%6d_%d.pcm",time_s.tv_sec,vol);
	fdp = open(name_file,O_RDWR|O_CREAT|O_TRUNC,0777);
	record_count ++;
#endif

    /* reset echo engine */
    echo_wakeup_reset(ew);

    /* callback function register */
    echo_wakeup_register_handler(ew, NULL, _wakeup_aec_callback);
	ret = echo_wakeup_start(ew, param);
    if(ret != 0)
    {
		PERROR("echo_wakeup_start failed! ,error = %d \n",ret);
        error_flag = true;
        goto agn_delete_exit;
    }

	ret = pipe_init();
	if (ret != 0) {
		error_flag = true;
		perror("pipe_init");
		goto agn_delete_exit;
	}
	ret = sound_device_init(volume_set);
	if (ret != 0) {
		printf(" sound_device_init failed,error = %d \n",ret);
		error_flag = true;
		goto ang_pipe_exit;
	}

	ret = sound_aec_enable();
	if (ret != 0) {
		error_flag = true;
		goto SOUND_DEV_DEINIT;
	} else{
		aec_able_status = true;
	}

   	aec_wakeup_flag = AEC_START;
//------------------------------------------------------- start dmic_read thread
	ret = pthread_create(&tid1, NULL, dmic_read, NULL);
	if (ret != 0){
		PERROR("pthread_create dmic_read failed,error = %d \n",ret);
	//	aec_wakeup_flag = AEC_WAKEUP_TID1_EXIT;
	    aec_wakeup_flag = AEC_END;
		error_flag = true;
		goto aec_end_working;
	}
//	pthread_detach(tid1);

//------------------------------------------------------- start loopback_read thread
	ret = pthread_create(&tid2, NULL, loopback_read, NULL);
	if (ret != 0){
		PERROR("pthread_create loopback_read failed,error = %d \n",ret);
		error_flag = true;
	    aec_wakeup_flag = AEC_END;
		goto aec_end_working;
	}
//	pthread_detach(tid2);

    printf("Please Speak(唤醒词：你好小乐) ...\n");
//------------------------------------------------------- start aec_handle thread
	DEBUG("-----------------------------------> Start aec_handle\n");
	while (AEC_WAKEUP != aec_wakeup_flag && AEC_END != aec_wakeup_flag
		&& !error_flag && !aec_end_flag) {
		status = read(fddmic[0], bufr, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == 0) {
				goto aec_end_working;
			} else {
				error_flag = true;
				goto aec_end_working;
			}
		}
		status = read(fdplay[0], bufp, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == 0) {
				goto aec_end_working;
			} else {
				printf("read fdplay[0] err not AEC_SIZE\n");
				error_flag = true;
				goto aec_end_working;
			}
		}
		#ifdef AEC_FILE_DEBUG
				write(fdr, bufr, AEC_SIZE);
				write(fdp, bufp, AEC_SIZE);
		#endif	//*/
		echo_wakeup_process(ew, bufr, bufp, AEC_SIZE);
	}
	aec_wakeup_flag = AEC_END;

aec_end_working:
	sound_wake_end();
	if (aec_able_status == true)
		sound_aec_disable();

SOUND_DEV_DEINIT:
	sound_device_release();
ang_pipe_exit:
    pipe_close();
agn_delete_exit:
	//if(no_aec)
	//	mozart_soundcard_uninit(record_info);

	if (ew){
		echo_wakeup_end(ew);
		echo_wakeup_reset(ew);
	}

#ifdef AEC_FILE_DEBUG
	close(fdr);
	close(fdp);
#endif
error_exit:
	g_aec_stop_flag = 0;
	ai_aec_working =0;
	if(error_flag == true)
    {
		ret = -1;
    }
    aec_end_flag = false;
	aec_wakeup_flag = AEC_IDEL;
	DEBUG("Stoped aec! \n");
	return ret;
}

