#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/soundcard.h>

#include "ai_error.h"
#include "ai_server.h"
#include "cJSON.h"

const char *error_type[AI_ERROR_MAX]={
	"NONE",
	"ATUHORITY",
	"INVALID_DOMAIN",
	"SYSTEM",
	"NO_VOICE",
	"SEM_FAIL_1",
	"SEM_FAIL_2",
	"SEM_FAIL_3",
	"SERVER_BUSY",
	"NET_SLOW",
	"NET_FAIL",
	"UNKNOW"
};

int ai_error_print(int error_id){
	switch(error_id){
		case 70504: PERROR("The requrest should be required in configure.\n"); break;
		case 70505: PERROR("The coreType should be required in params.\n"); break;
		case 70506: PERROR("Do not surpport the core type.(may be core new error)\n"); break;
		case 70507: PERROR("The params should be json string.\n"); break;
		case 70508: PERROR("Has not start or start error.\n"); break;
		case 70509: PERROR("The ctl channel error:\n"); break;
		case 70510: PERROR("Speech channel error:\n"); break;
		case 70511: PERROR("The configure should be json string\n"); break;
		case 70512: PERROR("The cloud module should be need.\n"); break;
		case 70513: PERROR("The configure neead UDSPath"); break;

		case  70601: PERROR("The params is not json.\n");break;
		case  70602: PERROR("The params has no request configure.\n");break;
		case  70603: PERROR("Websocket connect timeout.\n");break;
		case  70604: PERROR("Websocket connect error.\n");break;
		case  70605: PERROR("Websocket connect failed.\n");break;
		case  70606: PERROR("The auth information does not complete witch needs by websocket\n");break;
		case  70607: PERROR("New audioenc failed.\n");break;
		case  70608: PERROR("Start audioenc failed.\n");break;
		case  70609: PERROR("Receive server response, but not json.\n");break;
		case  70610: PERROR("Network abnormal.\n");break;
		case  70612: PERROR("The message type error for cloud.\n");break;
		case  70613: PERROR("Send timeout.\n");break;
		case  70614: PERROR("Wait server timeout.\n");break;
		case  70615: PERROR("Network is unreachable: ping time out.\n");break;
		case  70616: PERROR("The server configure should be json array\n");break;

		case 70701: PERROR("New ctl channel error.\n"); break;
		case 70702: PERROR("New thread error.\n"); break;
		case 70703: PERROR("New vad error.\n"); break;
		case 70704: PERROR("New speech channel error.\n"); break;
		case 70705: PERROR("Send speech channel rfd error.\n"); break;
		case 70706: PERROR("Write speech channel for recordId error.\n"); break;
		case 70707: PERROR("Write speech channel for usrId error.\n"); break;
		case 70708: PERROR("Write speech channel for cb error.\n"); break;
		case 70709: PERROR("Write speech channel for start error.\n"); break;
		case 70710: PERROR("Write ctl channel for vadVersion error.\n"); break;
		case 70711: PERROR("Write ctl channel for recordStartTime error.\n"); break;
		case 70712: PERROR("Speech channel not ready.\n"); break;
		case 70713: PERROR("Speech channel send feed data error.\n"); break;
		case 70714: PERROR("Vad feed error."); break;
		case 70715: PERROR("Ctl channel send vadStartIndex error.\n"); break;
		case 70716: PERROR("Ctl channel send vadStartTime error.\n"); break;
		case 70717: PERROR("Speech channel send feed data error.\n"); break;
		case 70718: PERROR("Ctl channel send recordEndTime error.\n"); break;
		case 70719: PERROR("Speech channel stop error.\n"); break;
		case 70720: PERROR("Ctl channel send WIFI error.\n"); break;
		case 70721: PERROR("Ctl channel send deviceId error.\n"); break;
		case 70722: PERROR("Ctl channel send result json error.\n"); break;
		case 70723: PERROR("Call sequence errors.\n"); break;
		case 70724: PERROR("Auth failed:\n"); break;
		case 70725: PERROR("Generate bin failed:\n"); break;
		case 70726: PERROR("Generate bin success\n"); break;
		case 70727: PERROR("new echo failed\n"); break;
		default:
			PERROR("Unknow Error.\n");
			break;
	}
	return error_id;
}

int ai_error_get_id(int error_id){
	int error = AI_ERROR_NONE;
	if (error_id == 0){
		goto exit;
	}
	if (ERROR_PRINT){
		ai_error_print(error_id);
	}
	switch (error_id){
		case  70604:
		case  70605:
			error = AI_ERROR_NET_FAIL;
			break;
		case  70603:
		case  70613:
			error = AI_ERROR_NET_SLOW;
			break;
		default:
			error = AI_ERROR_SERVER_BUSY;
			break;
	}

exit:
	return error;
}


