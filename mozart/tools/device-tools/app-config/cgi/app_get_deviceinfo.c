#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>

#include "cgi.h"
#include "cJSON.h"
#include "ota_interface.h"
#include "utils_interface.h"

int main(void)
{
	cJSON *pObject = cJSON_CreateObject();

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	//devicename
	cJSON_AddStringToObject(pObject,"deviceName","AIHOME");
	//hardware
	cJSON_AddStringToObject(pObject,"hardware","DS-WB38");
	//Software
	int fd =  mozart_ota_init();
	int externalVersion = mozart_ota_get_version(fd);
    cJSON_AddNumberToObject(pObject,"externalSoftware",externalVersion);

	//macAddr
	char macaddr[] = "255.255.255.255";
	memset(macaddr, 0, sizeof (macaddr));
	get_mac_addr("wlan0", macaddr, "");
    cJSON_AddStringToObject(pObject,"macAddr",macaddr+4);

	//ipaddr
    char *ipAddress = get_ip_addr("wlan0");
    cJSON_AddStringToObject(pObject,"ipaddr",ipAddress);

	//Electricity
    int power  = mozart_get_battery_capacity();
	cJSON_AddNumberToObject(pObject,"electricity",power);

	// 打印JSON数据包
    char *out = cJSON_Print(pObject);
 	printf("%s\n",out);
	// 释放内存
    cJSON_Delete(pObject);
    free(out);

	cgi_end();

	return 0;
}

