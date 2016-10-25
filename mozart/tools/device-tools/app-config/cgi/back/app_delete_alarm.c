#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

#include "cgi.h"
#include "cJSON.h"
#include "alarm_interface.h"

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	char* deleteId = cgi_param("deleteId");
	if (deleteId){
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~deleteId: %s\n", deleteId);
	}
	else {
		printf("~~~~~~~~~~~~~~~~ deleteId is error\n");
	}
	int deleId = atoi(deleteId);

	Alarm alarm;
	alarm.alarm_id = deleId;
	
	delete_alarm_mode_to_app(&alarm);
	//mozart_alarm_init_to_app();
	//mozart_alarm_list_load_from_file_to_app();
	//mozart_alarm_delete_alarm(deleId);
	//List *listalarm;
	//listalarm=mozart_alarm_get_alarm_list();
	//mozart_alarm_list_save_to_file(listalarm);
	
	cgi_end();

	return 0;
}


