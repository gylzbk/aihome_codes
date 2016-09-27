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


ai_aec_flag_t ai_aec_flag;

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
			ai_aec_flag.state = AEC_WAKEUP;
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
//	sound_wake_end();
	while(ai_aec_flag.state != AEC_IDEL){
		usleep(1000);		//	10ms*5000 = 50s
		if (++timeout > 50000){
			break;
		}
	}
	return 0;
}

void  ai_aec_stop(void){
	DEBUG("=================> Stop aec!\n");
	if (ai_aec_flag.state != AEC_IDEL){
		ai_aec_flag.set_end = true;
		ai_aec_stoping();
	}
}

#if 0 //def AEC_FILE_DEBUG
	int record_count;
	int fdr = -1;
	int fdp = -1;
#endif


int ai_aec(echo_wakeup_t *ew)
{
    int mode = 0;
    int ret = 0;
    pthread_t tid1 = 0;
    pthread_t tid2 = 0;
    pthread_t tid3 = 0;
    void *tret = NULL;
	ai_aec_flag.error = false;
    ai_aec_flag.state = AEC_START;
	ai_aec_flag.set_end = false;

	int aec_able_status = false;

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
        ai_aec_flag.error = true;
        goto agn_delete_exit;
    }

	ret = pipe_init();
	if (ret != 0) {
		ai_aec_flag.error = true;
		perror("pipe_init");
		goto agn_delete_exit;
	}
	ret = sound_device_init(VOLUME_AEC);
	if (ret != 0) {
		printf(" sound_device_init failed,error = %d \n",ret);
		ai_aec_flag.error = true;
		goto ang_pipe_exit;
	}

	ret = sound_aec_enable();
	if (ret != 0) {
		ai_aec_flag.error = true;
		goto SOUND_DEV_DEINIT;
	} else{
		aec_able_status = true;
	}

   	ai_aec_flag.state = AEC_START;
//------------------------------------------------------- start dmic_read thread
	ret = pthread_create(&tid1, NULL, dmic_read, NULL);
	if (ret != 0){
		PERROR("pthread_create dmic_read failed,error = %d \n",ret);
		ai_aec_flag.error = true;
		goto agn_end_exit;
	}


//------------------------------------------------------- start loopback_read thread
	ret = pthread_create(&tid2, NULL, loopback_read, NULL);
	if (ret != 0){
		PERROR("pthread_create loopback_read failed,error = %d \n",ret);
		ai_aec_flag.error = true;
	    ai_aec_flag.state = AEC_WAKEUP_TID1_EXIT;
	    pthread_join(tid1,&tret);
		goto agn_end_exit;
	}

//------------------------------------------------------- start loopback_read thread
	ret = pthread_create(&tid3, NULL, aec_handle, ew);
	if (ret != 0){
		PERROR("pthread_create aec_handle failed,error = %d \n",ret);
		ai_aec_flag.error = true;
		/* force other pthread exit */
	    ai_aec_flag.state = AEC_WAKEUP_TID2_EXIT;
	    pthread_join(tid2,&tret);
	    ai_aec_flag.state = AEC_WAKEUP_TID1_EXIT;
	    pthread_join(tid1,&tret);
		goto agn_end_exit;
	}
    printf("Please Speak(唤醒词：你好小乐) ...\n");

    /* waiting for AEC wakeup */
    pthread_join(tid3,&tret);
    ai_aec_flag.state = AEC_WAKEUP_TID2_EXIT;
    pthread_join(tid2,&tret);
    ai_aec_flag.state = AEC_WAKEUP_TID1_EXIT;
    pthread_join(tid1,&tret);

	sound_aec_disable();
	aec_able_status = false;


agn_end_exit:
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
	ai_aec_flag.set_end = false;
	ai_aec_flag.state = AEC_IDEL;
	DEBUG("Stoped aec! \n");
	return ai_aec_flag.error;
}
