#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cgi.h"
#include "wifi_interface.h"


#define MAXLINE 1024

int callback_handle_func(const char *p)
{
	printf(">>>>>>Hello,the change process of the network is %s<<<<<<\n",p);
	return 0;
}

int main(void)
{
	char *ap_cmd= NULL;

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	puts(""
	     "<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>"
	     "<html>"
	     "<head>"
	     "	"
	     " <meta http-equiv='content-type' content='text/html; charset=ISO-8859-1'>"
	     "	"
	     "   <meta name='author' content='Rafael Steil'>"
	     "    <title>LIBCGI Examples</title>"
	     "   </head>"
	     "  <body text='#000000' bgcolor='#ffffff' link='#0000ee' vlink='#551a8b' alink='#0000ee'>"		
	     "");

	// ap-cmd
	if (cgi_param("ap"))
		printf("ap: %s<br>", (ap_cmd = cgi_param("ap")));
	else
		puts("ap: Empty<br>");

	puts(""
	     "</body>"
	     "</html>"
	     "");	

	printf("result=yes\n");

	cgi_end();

#if 0
	bool status = false;
	struct client_register info;
	struct wifi_msg mode;

	info.pid = getpid();
	info.reset = 1;
	info.priority = 3;
	strcpy(info.name, "from_app");

	register_to_networkmanager(info, callback_handle_func);

	mode.force = true;
	strcpy(mode.name, "from_app");
	mode.cmd = SW_AP;

	status = request_wifi_mode(mode);
	printf("recv status is %d in %s:%s:%d\n", status, __FILE__, __func__, __LINE__);
#else
	system("startap.sh");
#endif

	return 0;
}
