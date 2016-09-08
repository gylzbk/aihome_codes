#include <stdio.h>
#include <string.h>

#include "wifi_interface.h"

int main(int argc,char *argv[])
{
	wifi_info_t infor = get_wifi_mode();

	printf("The current of wifi mode is %s, SSID is %s\n", wifi_mode_str[infor.wifi_mode],infor.ssid);

	return 0;
}
