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

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	//header
	puts("<meta http-equiv='content-type' content='text/html; charset=utf-8'>\n"
	     "<html>\n"
	     "<body style='OVERFLOW:SCROLL;OVERFLOW-X:HIDDEN'>\n");

	puts("<center>\n");
	puts("<form action='ap_config_confirm_page' method='get' enctype='text/plain'>\n");
	puts("    <table border='0'>\n");
	puts("        <tr>\n");
  	puts("			<th align='left' valign='middle'>SSID:</th>\n");
	puts("			<td align='left' valign='middle'>SmartAudio-<input type='text' style='width:50%' name='ssid'></td>\n");
	puts("		  </tr>\n");
	puts("		  <tr>\n");
	puts("			<th align='left' valign='middle'>密码: </th>\n");
	puts("			<td align='left' valign='middle'>\n");
	puts("				<input onkeyup='value=value.replace(/[\\W]/g,'') '\n");
	puts("				onbeforepaste='clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))'\n");
	puts("				onkeydown='if(event.keyCode==13)event.keyCode=9' type='password' style='width:100%; height:100%;' name='password'>\n");
	puts("			</td>\n");
	puts("		  </tr>\n");
	puts("		  <tr>\n");
	puts("			<th align='left' valign='middle' noWrap>\n");
	puts("				加密方式: \n");
	puts("			</th>\n");
	puts("			<td align='left' valign='middle'>\n");
	puts("				不加密<input type='radio' name='encrypt' value='no'>\n");
	puts("				加密<input type='radio' name='encrypt' value='yes' checked='checked'>\n");
	puts("			</td>\n");
	puts("		  </tr>\n");
	puts("		  <tr>\n");
	puts("			<th colspan=2 align='middle' valign='middle' style='width:100%, height:150%'>\n");
	puts("				<input type='submit' style='width:30%;' align='center' value='确定'>\n");
	puts("				<input type='reset' style='width:30%;' align='center' value='清空'>\n");
	puts("			</th>\n");
	puts("		  </tr>\n");
	puts("	  </table>\n");
	puts("</form>\n");
	puts("</center>\n");

	puts("<p>注意事项：</p>\n");
	puts("<p>1. SSID支持字母、数字和符号任意组合；</p>\n");
	puts("<p>2. 不加密模式下，密码项无效（此时可留空）；</p>\n");
	puts("<p>3. 为保证安全性，加密模式采用wpa2方式；</p>\n");
	puts("<p>4. 加密模式下，密码长度范围为8-63位之间(含８位和63位)，可由数字、字母、符号组成；</p>\n");
	puts("<p>5. 加密模式下，SSID和密码任一项留空或者不满足要求时，不会做任何操作；</p>\n");

	puts("\t</body>\n"
	     "</html>\n");

	cgi_end();

	return 0;
}
