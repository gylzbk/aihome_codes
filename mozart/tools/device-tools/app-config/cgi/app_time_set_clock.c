#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "cgi.h"

int main(void)
{
        cgi_init();
        cgi_process_form();
        cgi_init_headers();

        int time = 0;
        char* ctime = cgi_param("time");
        if (ctime){
                time = atoi(ctime);
                printf("ctime = %s\n", ctime);
                printf("time = %d", time);
        } else {
                printf("set time error");
                return -1;
        }

        struct timeval tv;
        tv.tv_sec = time;

        settimeofday(&tv,NULL);

        usleep(100);
        system("hwclock -w");
        usleep(100);

        cgi_end();

        return 0;
}
