#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cgi.h"

#define TIMER_FILE_PATH "/tmp/set_timer.timer"

typedef void (*FUNC)(int);

static void power_off(int signo)
{
        system("poweroff");
}

static void _cancle_timer(int arg)
{
        alarm(0);
        system("rm -rf /tmp/timer.pid");
        system("rm -rf /tmp/set_timer.timer");

        exit(0);
}

static void read_time(void)
{
	FILE *fd;
	if ((fd = fopen(TIMER_FILE_PATH, "r")) ==NULL)
		printf("error open file\n");
	int timestamp = 0;
	fread(&timestamp,sizeof(int),1,fd);
	printf("timesize is %d\n",timestamp);
	fclose(fd);
}

static void save_timer(int seconds)
{
	FILE *fd;
	struct timeval tv;
	if ((fd = fopen(TIMER_FILE_PATH, "wb+")) == NULL)
		printf("error open file\n");
	if(gettimeofday(&tv,NULL)==-1)
	{
		printf("error get time \n");
	}
	int timestamp = tv.tv_sec + seconds;
	fwrite(&timestamp,sizeof(int),1,fd);
	fclose(fd);
	read_time();
}

static void cancle_timer(void)
{
        if (!access("/tmp/timer.pid", 0) &&
            !access("/tmp/set_timer.timer", 0)) {
                int fd = -1;
                char buf[32] = {};

                fd = open("/tmp/timer.pid", O_RDONLY);
                if (fd < 0) {
                        printf("open /tmp/timer.pid error: %s, exit...\n", strerror(errno));
                        return;
                }

                read(fd, buf, 32);
                close(fd);

                kill(atoi(buf), SIGUSR1);
        }
}

static void set_timer(int seconds)
{
        char buf[32] = {};
        int fd = -1;

        sprintf(buf, "%d", getpid());
        fd = open("/tmp/timer.pid", O_CREAT | O_RDWR);
        if (fd < 0) {
                printf("create /tmp/timer.pid error: %s, exit...\n", strerror(errno));
                return;
        }

        write(fd, buf, strlen(buf)); //TODO: check ret
        close(fd);

        signal(SIGALRM, power_off);
        signal(SIGUSR1, _cancle_timer);
        alarm(seconds);
}

int main(int argc, char **argv)
{
	int pid = -1;
        int settime = 0;

	cgi_init();
	cgi_process_form();
	cgi_init_headers();

        char* ctimer = cgi_param("timer");
	if (ctimer){
		settime = atoi(ctimer);
		printf("ctimer = %s\n", ctimer);
		printf("settime = %d", settime);
	}else{
		printf("set time error");
	}

        // cancle timer action.
        if (settime == 0) {
                cancle_timer();
		cgi_end();
                return 0;
        }

        // set a new timer.
	pid = fork();
	if (pid < 0) {
		printf("fork for poweroff timer error: %s, exit...\n", strerror(errno));
		return -1;
	} else if (pid > 0) {
		cgi_end();
                return 0;
	} else {
                // cancle previous timer if exist.
                cancle_timer();
                set_timer(settime);
		save_timer(settime);
                while(1);
	}

        printf("%s exit...\n", argv[0]);

        return 0;
}
