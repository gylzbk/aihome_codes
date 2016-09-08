#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "cgi.h"
#include "wifi_interface.h"

#define AP_FILE "/usr/data/apinfo.ini"

#define AP_CONF \
	"[ap]\n" \
	"encrypt=%s\n" \
	"SSID=%s-%s\n" \
	"PASSWD=%s\n"

int callback_handle_func(const char *p)
{
	printf(">>>>>>Hello,the change process of the network is %s<<<<<<\n",p);
	return 0;
}


int main(void)
{
	int ap_fd = -1;
	char tmp[512] = {};
	char hostname[16] = "Unknown";

	char *ssid = NULL;
	char *password = NULL;
	char *encrypt = NULL;

	gethostname(hostname, sizeof(hostname) - 1);

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	//header
	puts("<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>\n"
	     "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0'/>\n"
	     "<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n"
	     "<html>\n"
	     "<body text='#000000' bgcolor='#ffffff' link='#0000ee' vlink='#551a8b' alink='#0000ee'>\n");


	// parse ap info
	ssid = cgi_param("ssid");
	password = cgi_param("password");
	encrypt = cgi_param("encrypt");

	if (!ssid) {
		puts("SSID未输入，请到<a href='javascript:history.go(-1)' target=_self>ap配置界面</a>重新配置</br>\n");
		goto exit_direct;
	}

	if (encrypt) {
		if (!password) {
			puts("未输入密码，请到<a href='javascript:history.go(-1)' target=_self>ap配置界面</a>重新配置</br>\n");
			goto exit_direct;
		} else if (strlen(password) < 8) {
			puts("密码长度不合法(小于8位)，请到<a href='javascript:history.go(-1)' target=_self>ap配置界面</a>重新配置</br>\n");
			goto exit_direct;
		} else if (strlen(password) > 63) {
			puts("密码长度不合法(大于63位)，请到<a href='javascript:history.go(-1)' target=_self>ap配置界面</a>重新配置</br>\n");
			goto exit_direct;
		}
	}

	printf("新AP配置成功,正在切换...\n");

//save_ap_info
	ap_fd = open(AP_FILE, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if (ap_fd < 0) {
		printf("save ap info fail: %s\n", strerror(errno));
			goto exit_direct;
	}

	sprintf(tmp, AP_CONF, encrypt, hostname, ssid, password);
	write(ap_fd, tmp, strlen(tmp));

	close(ap_fd);

//switch_ap_mode
	struct wifi_client_register info;
	wifi_ctl_msg_t mode;

	info.pid = getpid();
	info.reset = 1;
	info.priority = 3;
	strcpy(info.name, "web_config");

	register_to_networkmanager(info, callback_handle_func);

	mode.force = true;
	mode.cmd = SW_AP;

	request_wifi_mode(mode);

exit_direct:

	puts("</body>\n"
	     "</html>\n");
	cgi_end();

	return 0;
}
