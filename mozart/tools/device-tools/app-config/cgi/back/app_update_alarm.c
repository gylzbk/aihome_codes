#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "cgi.h"
#include "cJSON.h"
#include "alarm_interface.h"
//#include "alarms.h"

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	char* data = cgi_param("data");
	if (data){
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~data: %s\n", data);
	}
	else {
		printf("~~~~~~~~~~~~~~~~ data is error\n");
	}

	cJSON* pItem = cJSON_Parse(data);

	int mId = cJSON_GetObjectItem(pItem, "id")->valueint;
	int mStartHour = cJSON_GetObjectItem(pItem, "startHour")->valueint;
	int mStartMinute = cJSON_GetObjectItem(pItem, "startMinute")->valueint;	
	int mEnabled = cJSON_GetObjectItem(pItem, "enabled")->valueint;      
	int mVolume = cJSON_GetObjectItem(pItem, "volume")->valueint;
	int mRecurrence = cJSON_GetObjectItem(pItem, "recurrence")->valueint; 
	int weekdays_active[7] = {0, 0, 0, 0, 0, 0, 0};
	int i = 0;
	while(mRecurrence > 0){
		 weekdays_active[i]=mRecurrence%2;
         i=i+1;
         mRecurrence=mRecurrence/2;
	}
	
	char* mprogramData = cJSON_GetObjectItem(pItem, "programData")->valuestring;
	char* mprogramUrl = cJSON_GetObjectItem(pItem, "programUrl")->valuestring;
	///printf("programUrl = %s\n", mprogramUrl);
	//printf("programData = %s\n", mprogramData);

	mozart_alarm_init_to_app();
	mozart_alarm_list_load_from_file_to_app();
	//mozart_alarm_delete_alarm(mId);
	Alarm alarm;
	alarm.hour = mStartHour;
	alarm.minute = mStartMinute;
	alarm.weekdays_active[0] = weekdays_active[0];
	alarm.weekdays_active[1] = weekdays_active[1];
	alarm.weekdays_active[2] = weekdays_active[2];
	alarm.weekdays_active[3] = weekdays_active[3];
	alarm.weekdays_active[4] = weekdays_active[4];
	alarm.weekdays_active[5] = weekdays_active[5];
	alarm.weekdays_active[6] = weekdays_active[6];
	alarm.enabled = mEnabled;
	alarm.timestamp = 12;
	alarm.alarm_id = mId;
	alarm.volume = mVolume;
	memcpy(alarm.programData, mprogramData, sizeof(char)*256);
	memcpy(alarm.programUrl, mprogramUrl, sizeof(char)*128);
	printf("programUrl = %s\n", alarm.programUrl);
	printf("programData = %s\n", alarm.programData);

	printf("mStartHour=%d, mStartMinute=%d, mEnabled=%d, mId=%d\n", alarm.hour,alarm.minute,alarm.enabled,alarm.alarm_id);
	int j = 0;
	for(; j < 7; j++){
		printf("weekdays_active[%d] = %d ", j, weekdays_active[j]);
	}

	update_alarm_mode_to_app(&alarm);
	//List *listalarm;
	//listalarm=mozart_alarm_get_alarm_list();
	//mozart_alarm_list_save_to_file(listalarm);

	cgi_end();

	return 0;
}


