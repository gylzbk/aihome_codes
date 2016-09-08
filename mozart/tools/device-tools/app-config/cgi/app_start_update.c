#include <stdio.h>
#include <string.h>
#include "cgi.h"
#include "utils_interface.h"
#include "ota_interface.h"
#include "tips_interface.h"

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	//在线升级

	char* otaUrl = "";
	otaUrl = cgi_param("otaUrl");
	if (otaUrl){
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~otaUrl: %s\n", otaUrl);
		mozart_play_key("update_found");
	}
	else {
		printf("~~~~~~~~~~~~~~~~ ota url is error\n");
		mozart_play_key("update_not_found");
	}

	unsigned int urlsize = strlen(otaUrl);
	int fd =  mozart_ota_init();

	mozart_ota_seturl(otaUrl, urlsize, fd);

	mozart_play_key("updateing");
	mozart_ota_start_update();

	cgi_end();

	return 0;
}
