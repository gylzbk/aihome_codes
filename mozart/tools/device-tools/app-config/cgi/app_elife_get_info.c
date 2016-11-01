#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "cgi.h"
#include "elife_doss.h"
char *send_c =
"{\
\"type\": \"info\",\
\"input\": \"\",\
\"sessionid\": \"\"\
}";

int main(void)
{
    int ret = 0;
	bool is_register = false;
	char result[1024] = {0};

    cgi_init();
    cgi_process_form();
    cgi_init_headers();


	memset(result, 0, sizeof(result));
	ret = get_info(result);
//	printf("send = %s<br>\n", send_c);
//	ret = send_wise_voice_cmd(send_c, result);

	if (ret){
		printf("get info error!<br>\n");
		printf("error id = %d<br>\n",ret);
		return -1;
	}
	if (strlen(result) != 0){
		printf("%s<br>\n",result);
	}

#if 0
	if (is_register){
		printf("it was registed elife!<br>\n");
	} else {

		printf("it wasn't registed elife!<br>\n");
	}#endif

    cgi_end();
#endif
    return 0;
}
