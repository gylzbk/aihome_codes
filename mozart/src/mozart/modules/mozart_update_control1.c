#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "ini_interface.h"
#include "tips_interface.h"
#include "updater_interface.h"

void *mozart_updater_check_version(void *arg)
{
	nvinfo_t *nvinfo = NULL;

	pthread_detach(pthread_self());

	while(1) {
		if ((nvinfo = mozart_updater_chkver()) != NULL) {
			free(nvinfo);
			system("updater -p");
			break;
		}
		sleep(1);
	}

	return NULL;
}

int mozart_update_control_startup(void)
{
	pthread_t updater_chkver_pthread;

	if (pthread_create(&updater_chkver_pthread, NULL, mozart_updater_check_version, NULL) == -1) {
		printf("Create updater check version pthread failed: %s.\n", strerror(errno));
		return -1;
	} else {
		return 0;
	}
}
