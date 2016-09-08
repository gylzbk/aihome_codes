#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>

#include "cgi.h"
#include "alarm_interface.h"

#define TIMER_FILE_PATH "/tmp/set_timer.timer"

int read_time()
{
        FILE *fd;
        int timestamp = 0;

        if ((fd = fopen(TIMER_FILE_PATH, "r")) ==NULL) {
                return -1;
        }

        fread(&timestamp,sizeof(int),1,fd);
        //printf("timestamp is %d\n",timestamp);
        fclose(fd);

        return timestamp;
}

int main(void)
{
        cgi_init();
        cgi_process_form();
        cgi_init_headers();

        int timerstamp=read_time();
        int nowtimestamp = mozart_time_get_clock_timestamp();

        if (timerstamp == -1)
                printf("-1\n");
        else
                printf("%d\n",timerstamp-nowtimestamp);

        cgi_end();

        return 0;
}

