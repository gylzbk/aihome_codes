#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "cgi.h"
#define USER_INFO_PATH "/usr/data/user_info.conf"


int main(void)
{
    int i = 0;
    char *mobile = NULL;
    char *pwd = NULL;
	char *data = NULL;
	char *cmd = NULL;

    cgi_init();
    cgi_process_form();
    cgi_init_headers();

    // mobile
	mobile = cgi_param("mobile");
	if (mobile){
      	 printf("mobile: %s<br>", mobile);
    } else {
        printf("mobile=null<br>");
        cgi_end();
        return -1;
    }
	// pwd
	pwd = cgi_param("pwd");
	if (pwd){
		printf("pwd: %s<br>", pwd);
    } else {
        printf("pwd=null<br>");
        cgi_end();
        return -1;
    }


    cmd = malloc(strlen(mobile)+strlen(pwd)+strlen(USER_INFO_PATH)+20);
	if (cmd == NULL){
		cgi_end();
		return -1;
	}
	//  rm /usr/data/user_info.conf
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"rm %s",USER_INFO_PATH);
	system(cmd);

	//  touch /usr/data/user_info.conf
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"touch %s",USER_INFO_PATH);
	system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"echo mobile=%s >> %s", mobile, USER_INFO_PATH);
	system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"echo pwd=%s >> %s", pwd, USER_INFO_PATH);
	system(cmd);

    printf("register elife successful!<br>\n");
	free(cmd);

    cgi_end();

    return 0;
}
