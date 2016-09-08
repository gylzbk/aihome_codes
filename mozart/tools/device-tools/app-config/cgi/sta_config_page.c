#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "linklist_interface.h"
#include "cgi.h"

int malloc_cnt = 0;
int free_cnt = 0;

typedef struct wifi {
	char *ssid;
	int level;
	int channel;
	char *encryption;
} wifi;

int parse_wifilist(char *file, List *list)
{
	FILE *fp = NULL;
	char buf[512] = {};
	int size = 0;
	wifi *new_wifi;

	fp = fopen(file, "rb");
	if (fp == NULL) {
		printf("open %s error: %s\n", file, strerror(errno));
		return 1;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf)] = '\0';

		//find a new wifi, alloc a new list data and insert to list.
		if (strstr(buf, "Cell")) {
			//printf("malloc for newwifi: %d\n", malloc_cnt++);
			new_wifi = malloc(sizeof(wifi));
			new_wifi->ssid = NULL;
			new_wifi->channel = -1;
			new_wifi->level = -1;
			new_wifi->encryption = "Unknown";

			list_insert(list, new_wifi);
		}

		//parse ssid.
		if (strstr(buf, "ESSID:\"")) {
			size = strlen(strstr(buf, "ESSID:\"")) - strlen("ESSID:\"") - strlen("\"\n");
			//printf("malloc for wifi->ssid: %d\n", malloc_cnt++);
			new_wifi->ssid = malloc(size + 1);
			memset(new_wifi->ssid, 0, size + 1);

			strncpy(new_wifi->ssid, strstr(buf, "ESSID:") + strlen("ESSID:\""), size);
		}

		//parse channel
		if (strstr(buf, "Channel")) {
			new_wifi->channel = atoi(strstr(buf, "Channel ") + strlen("Channel "));
		}

		//parse encryption
		if (strstr(buf, "Encryption key:on")) {
			//encryption wifi, check encryption method deeply later.
		} else if (strstr(buf, "Encryption key:off")) {
			new_wifi->encryption = "open";
		}

		//parse wpa
		if (strstr(buf, "IE:") && strstr(buf, "WPA ")) {
			new_wifi->encryption = "wpa";
		}

		//parse wpa2
		if (strstr(buf, "IE:") && strstr(buf, "WPA2 ")) {
			if(!strcmp(new_wifi->encryption, "wpa"))
				new_wifi->encryption = "wpa/wpa2";
			else
				new_wifi->encryption = "wpa2";
		}

		//parse signal level
		if (strstr(buf,"Signal level=")) {
			new_wifi->level = atoi(strstr(buf,"Signal level=") + strlen("Signal level="));
		}
	}

	fclose(fp);
	return 0;
}

#define CHECKBOX "<table border='0' style='width:100%, height:100%'>" \
	     "<tr>\n" \
	     "\t<td align='left' valign='middle' style='width:100%; height:100%;'>%s</td>\n" \
	     "\t<td nowrap align='middle' valign='middle' style='width:100%; height:100%;'>%s</td>\n" \
	     "\t<td nowrap align='middle' valign='middle' style='width:100%; height:100%;'>%d%%</td>\n" \
	     "\t<td nowrap align='right' valign='middle'><input type='radio' name='wifi' value='%s_%s' checked='%s'></td>\n" \
	     "</tr>\n" \
	     "</table>\n<hr/>\n"

static bool has_checked=false;
static void print_wifi_info(void *data)
{
	char buf[1024] = {};
	wifi *wf = (wifi *)data;
	char *checked = "no";

	if (!has_checked) {
		has_checked=true;
		checked = "yes";
	}

	sprintf(buf, CHECKBOX, wf->ssid, wf->encryption, wf->level, wf->ssid, wf->encryption, checked);
	puts(buf);

	return;
}

int compare_level(const void *s1, const void *s2)
{
	wifi *wifi1 = (wifi *)s1;
	wifi *wifi2 = (wifi *)s2;

	return (wifi2->level - wifi1->level);
}

int compare_ssid(const void *s1, const void *s2)
{
	wifi *wifi1 = (wifi *)s1;
	char *ssid = (char *)s2;

	return strcmp(wifi1->ssid, ssid);
}

int compare_channel(const void *s1, const void *s2)
{
	wifi *wifi1 = (wifi *)s1;
	wifi *wifi2 = (wifi *)s2;

	return (wifi1->channel - wifi2->channel);
}

void destory(void *s)
{
	wifi *wf = (wifi *)s;
	if (wf) {
		//printf("free wifi->ssid: %d\n", free_cnt++);
		if(wf->ssid)
			free(wf->ssid);
		wf->ssid = NULL;

		wf->level = -1;
		wf->channel = -1;
		wf->encryption = NULL;

		//printf("free wifi: %d\n", free_cnt++);
		free(wf);
	}

	wf = NULL;
}


void wifi_filter(List *list, List *new_list)
{
	int i = 0;
	int cnt = list_get_length(list);
	wifi *wf = NULL;
	wifi *new_wf = NULL;

	for (i = 0; i < cnt; i++) {
		wf = list_get_element(list, i);

		//if NULL, ignore it.
		if(!wf)
			continue;

		if(list_search(new_list, wf->ssid, &compare_ssid))
			continue;

		//else make a node to insert to list.
		new_wf = malloc(sizeof(wifi));
		memset(new_wf, 0, sizeof(wifi));

		if(wf->ssid) {
			new_wf->ssid = malloc(strlen(wf->ssid) + 1);
			memset(new_wf->ssid, 0, strlen(wf->ssid) + 1);
			strncpy(new_wf->ssid, wf->ssid, strlen(wf->ssid));
		}
		new_wf->channel = wf->channel;
		new_wf->level = wf->level;
		new_wf->encryption = wf->encryption;

		list_insert(new_list, new_wf);
	}
}

static void choice_wifi_manual(void)
{
        puts("<center>\n");
        printf("没有扫描到wifi, 请手动配置</br></br>\n");

        // form start
        printf("<form action='config_sta' method='get' enctype='text/plain'>\n"
               "<table border='0'>\n");
        // ssid
        printf("<tr>\n"
               "\t<th align='left' valign='middle'>SSID: </th>\n"
               "\t<td align='left' valign='middle'>\n"
               "\t<input type='text' name='ssid'/></td>\n"
               "</tr>\n"
               "\n");

        // passwd
        printf("<tr>\n"
               "\t<th align='left' valign='middle'>密码: </th>\n"
               "\t<td align='left' valign='middle'>\n"
               "\t<input type='text' name='passwd'/></td>\n"
               "</tr>\n"
               "\n");

        // submit
        printf("<th colspan=2 align='middle' valign='middle' style='width:100%, height:100%'>\n"
               "\t<input type='submit' style='width:30%;' align='center' value='确定'>\n"
               "\t<input type='reset' style='width:30%;' align='center' value='清空'>\n"
               "</th>\n");

        // form stop
        printf("</table>\n"
               "</br>\n"
               "</form>");
        puts("</center>\n");

        puts("<p>注意事项：</p>\n");
        puts("<p>1. SSID由字母、数字和符号组成；</p>\n");
        puts("<p>2. 如果要连接的wifi为开放网络，密码请留空；</p>\n");

        return;
}

static void choice_wifi_auto(List *list)
{
	//header
	puts("<meta http-equiv='content-type' content='text/html; charset=utf-8'>"
	     "<meta http-equiv='refresh' content='10'/>\n"
	     "<html>\n"
	     "<body style='OVERFLOW:SCROLL;OVERFLOW-X:HIDDEN'>\n");

	//form
	puts("<center>\n");
	printf("<form action='sta_config_confirm_page' method='get' enctype='text/plain'>");

	//table start
	puts("<table border='0'>\n");

	//show wifi list
    list_traverse(list, &print_wifi_info);

	//table stop
	puts("</table>");

	// submit button
	puts("<input nowrap align='middle' valign='middle' style='width:100%;' type=button value=刷新 onclick=history.go(-0)>");
	puts("<input nowrap type='submit' style='width:100%;' align='center' value='确定'>");

	//form stop
	puts("</form>");
	puts("</center>\n");

	//html stop
	puts("</body>\n</html>\n");

    return;
}

int main(int argc, char **argv)
{
	int i = 0;
	List list;
	List new_list;

	system("iwlist scanning >/tmp/wifilist");

	/* linklist init */
	list_init(&list);
	list_init(&new_list);

	parse_wifilist("/tmp/wifilist", &list);

	list_sort(&list, compare_level);

	wifi_filter(&list, &new_list);

	cgi_init();
	cgi_init_headers();

    if (is_empty(&new_list))
            choice_wifi_manual();
    else
            choice_wifi_auto(&new_list);

	cgi_end();

	list_destroy(&list, &destory);

	list_destroy(&new_list, &destory);

	return 0;
}
