
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>

#include "ai_server.h"


#include "ai_elife_doss.h"
#define TONE_PATH "/usr/fs/usr/share/vr/tone/"
ai_elife_t ai_elife;

void ai_elife_init(void){
	ai_elife.name = NULL;
	ai_elife.position = NULL;
	ai_elife.cmd = NULL;
	ai_elife.movie.name = NULL;
	ai_elife.movie.sequence= -1;
	ai_elife.movie.director= NULL;
	ai_elife.movie.player= NULL;
	ai_elife.movie.type= NULL;
	ai_elife.movie.area= NULL;
}

void ai_elife_command_free(void){
	free(ai_elife.name);
	ai_elife.name= NULL;
	free(ai_elife.position);
	ai_elife.position= NULL;
	free(ai_elife.cmd);
	ai_elife.cmd= NULL;
}

void ai_elife_movie_free(void){
	free(ai_elife.cmd);
	ai_elife.cmd= NULL;
	free(ai_elife.movie.name);
	ai_elife.movie.name= NULL;
	free(ai_elife.movie.director);
	ai_elife.movie.director= NULL;
	free(ai_elife.movie.player);
	ai_elife.movie.player= NULL;
	free(ai_elife.movie.type);
	ai_elife.movie.type= NULL;
	free(ai_elife.movie.area);
	ai_elife.movie.area= NULL;
	ai_elife.movie.sequence= -1;
}

void ai_elife_free(void){
	ai_elife_command_free();
	ai_elife_movie_free();
}

int ai_elife_command(void){
	int result_num = 0;
	int error = 0;
	char play_voice[500] = {0};
	DEBUG("Start elfe command!...\n");
	if((ai_elife.name == NULL)
	// ||(ai_elife.position == NULL)
	 ||(ai_elife.cmd == NULL)){
		error= -1;
		PERROR("It's n't elife command!...\n");
		goto exit;
	}

	printf("name = %s\n",ai_elife.name);
	printf("position = %s\n",ai_elife.position);
	printf("cmd = %s\n",ai_elife.cmd);

	ai_elife.play_voice = (char *)play_voice;
	result_num   = send_wise_ctl_cmd(ai_elife.name, ai_elife.position, ai_elife.cmd, ai_elife.play_voice);
	DEBUG("result = %d, tts = %s\n",result_num,ai_elife.play_voice);
#if 1	// TTS play_voice
	ai_tts(ai_elife.play_voice,false);
#else
	char *result = NULL;
	switch(result_num){
	case _ELIFE_DOSS_OK:
		result = "elife_successful";
		break;
	case _ELIFE_DOSS_ERR_LOGIN:
		result ="elife_error_longin";
		break;
	case _ELIFE_DOSS_ERR_PARAM:
		result ="elife_error_slot";
		break;
	case _ELIFE_DOSS_ERR_DEVICE:
		result = "elife_error_unknow_device";
		break;
	case _ELIFE_DOSS_ERR_MODE:
		result = "elife_error_unknow_mode";
		break;
	case _ELIFE_DOSS_ERR_SUPPORT:
		result = "elife_error_unknow_control";
		break;
	case _ELIFE_DOSS_ERR_CMD:
		result = "elife_error_unknow_command";
		break;
	case _ELIFE_DOSS_ERR_NET:
		result = "elife_error_net";
		break;
	case _ELIFE_DOSS_ERR_SYSTEM:
		result = "elife_error_system";
		break;
	default:
		break;
	}
	DEBUG("result = %d,%s\n",result_num,ai_elife.play_voice);
	if (result){
		mozart_prompt_tone_key_sync(result, false);
	}
#endif

exit:
	ai_elife_command_free();
	return error;
}
int ai_elife_movie(void){
	int result_num = 0;
	int error = 0;
	char play_voice[500] = {0};
	DEBUG("Start elfe movie!...\n");
	printf("movie.cmd = %s\n",ai_elife.cmd);
	printf("movie.name = %s\n",ai_elife.movie.name);
	printf("movie.sequence = %d\n",ai_elife.movie.sequence);
	printf("movie.director = %s\n",ai_elife.movie.director);
	printf("movie.player = %s\n",ai_elife.movie.player);
	printf("movie.type = %s\n",ai_elife.movie.type);
	printf("movie.area = %s\n",ai_elife.movie.area);
	ai_elife.play_voice = (char *)play_voice;
	result_num   = send_wise_movie_cmd(ai_elife.cmd,
					ai_elife.movie.name,
					ai_elife.movie.sequence,
					ai_elife.movie.director,
					ai_elife.movie.player,
					ai_elife.movie.type,
					ai_elife.movie.area,
					ai_elife.play_voice);	//*/

	DEBUG("result = %d, tts = %s\n",result_num,ai_elife.play_voice);
	ai_tts(ai_elife.play_voice,false);
exit:
	ai_elife_movie_free();
	return error;
}



