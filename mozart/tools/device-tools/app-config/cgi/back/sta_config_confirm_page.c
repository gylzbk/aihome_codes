#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cgi.h"

#define MAXLINE 1024

int main(void)
{
	char *wifi = NULL;
	char *sep = NULL;

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	puts(""
	     "<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>"
	     "<html>"
	     "<head>"
	     "	"
	     " <meta http-equiv='content-type' content='text/html; charset=utf-8'>"
	     "	"
	     "   <meta name='author' content='Rafael Steil'>"
	     "    <title>wifi配置结果确认页面...</title>"
	     "   </head>"
	     "  <body text='#000000' bgcolor='#ffffff' link='#0000ee' vlink='#551a8b' alink='#0000ee'>"
	     "");

	// wifi
	wifi = cgi_param("wifi");

	if (!wifi) {
		puts("wifi选择有误，请到<a href='javascript:history.go(-1)' target=_self>sta配置界面</a>重新设置</br>\n");
		puts("</body>\n"
		     "</html>\n");
		goto exit_direct;
	}

	sep = strrchr(wifi, '_');
	if (!sep) {
		puts("内部错误，请联系我们</br>\n");
		puts("</body>\n"
		     "</html>\n");
		goto exit_direct;
	}
	*sep = '\0';

	printf("<form action='config_sta' method='get' enctype='text/plain'>\n"
	       "<table border='0'>\n");

	printf("<tr>\n"
	       "\t<th align='left' valign='middle'>SSID:</th>\n"
	       "\t<td align='left' valign='middle'>\n"
	       "\t<input name='ssid' readonly='readonly' value='%s'/></td></br>\n"
	       "</tr>\n"
	       "\n", wifi);

	if (!strcmp(sep, "_open")) {
		printf("<tr>\n"
		       "\t<th align='left' valign='middle'>密码:</th><br>\n"
		       "\t<td align='left' valign='middle'>开放网络,无需密码,请点击确认按钮</td></br>\n"
		       "</tr>\n"
		       "\n");
	} else {
		printf("<tr>\n"
		       "\t<th align='left' valign='middle'>密码: </th>\n"
		       "\t<td align='left' valign='middle'>\n"
		       "\t<input type='text' name='passwd'/></td>\n"
		       "</tr>\n"
		       "\n");
	}
	printf("<th colspan=2 align='middle' valign='middle' style='width:100%, height:100%'>\n"
	       "\t<input type='submit' style='width:30%;' align='center' value='确定'>\n"
	       "\t<input type='reset' style='width:30%;' align='center' value='清空'>\n"
	       "</th>\n"
	       "</table>\n"
	       "</br>\n"
	       "</form>");

exit_direct:
	cgi_end();

	//network_manager添加接口,可以选择连接到哪个wifi(前提是wifi已经配置好)

	return 0;
}
