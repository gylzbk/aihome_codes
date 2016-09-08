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
#define WPA_CONF "/usr/data/wpa_supplicant.conf"

int main(void)
{
	int wpa_conf_fd;
	char ssid_name[MAXLINE] = {};
	char psk[MAXLINE] = {};
	char *buf = "ctrl_interface=/var/run/wpa_supplicant\n"
		"ap_scan=1\n"
		"network={\n";

	char *ssid = NULL;
	char *passwd = NULL;

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	puts(""
	     "<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>\n"
	     "<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n"
	     "<html>\n"
	     "<body text='#000000' bgcolor='#ffffff' link='#0000ee' vlink='#551a8b' alink='#0000ee'>\n");

	// ssid
	ssid = cgi_param("ssid");
	if (!ssid) {
		puts("SSID未输入，请到<a href='javascript:history.go(-1)' target=_self>sta配置界面</a>重新配置</br>\n");
		puts("</body>\n"
		     "</html>\n");
		cgi_end();
		return -1;
	}
	sprintf(ssid_name, "ssid=\"%s\"\n", ssid);

	// passwd
	passwd = cgi_param("passwd");
	if (!passwd) {
		sprintf(psk, "key_mgmt=NONE\n");
	} else {
		if (strlen(passwd) < 8) {
			puts("密码长度不合法(小于8位)，请到<a href='javascript:history.go(-1)' target=_self>sta配置界面</a>重新配置</br>\n");
			puts("</body>\n"
			     "</html>\n");
			cgi_end();
			return -1;
		} else if (strlen(passwd) > 63) {
			puts("密码长度不合法(大于63位)，请到<a href='javascript:history.go(-1)' target=_self>sta配置界面</a>重新配置</br>\n");
			puts("</body>\n"
			     "</html>\n");
			cgi_end();
			return -1;
		}

		sprintf(psk, "psk=\"%s\"\n", passwd);
	}

	wpa_conf_fd = open(WPA_CONF, O_CREAT | O_WRONLY | O_TRUNC, 0x644);
	if (wpa_conf_fd < 0) {
		perror("open file fail");
		exit(-1);
	}

	write(wpa_conf_fd, buf, strlen(buf));
	write(wpa_conf_fd, ssid_name, strlen(ssid_name));
	write(wpa_conf_fd, psk, strlen(psk));
	write(wpa_conf_fd, "}", 1);

	close(wpa_conf_fd);

	printf("result=yes");

	puts("</body>"
	     "</html>");

	cgi_end();

	struct wifi_client_register info;
	wifi_ctl_msg_t mode;

	info.pid = getpid();
	info.reset = 1;
	info.priority = 3;
	strcpy(info.name, "web_config");

	register_to_networkmanager(info, NULL);

	mode.force = true;
	mode.cmd = SW_STA;

	request_wifi_mode(mode);

	return 0;
}
