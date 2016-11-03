#include <stdio.h>
#include <stdlib.h>

#include "nvrw_interface.h"

int main(int argc, char **argv)
{
	nvinfo_t *info = NULL;
	char *method = NULL;
	info = mozart_nv_get_nvinfo();

	if (!info) {
		printf("get nvinfo failed.\n");
		return -1;
	} else if (strcmp(info->magic, "OTA")) {
		printf("Invalid nv area.\n");
		free(info);
		return -1;
	}

	if (info->update_method.method == UPDATE_ONCE)
		method = "UPDATE_ONCE";
	else if (info->update_method.method == UPDATE_TIMES)
		method = "UPDATE_TIMES";
	else
		method = "UnKnown";


	printf("[nvimage]\n"
		   "current_version = v%s\n"
		   "update_version = v%s\n"
		   "update_flag = %d\n"
		   "update_process = %d\n"
		   "method = %s\n"
		   "storage = %s\n"
		   "location = %s\n"
		   "product = %s\n"
		   "url = %s\n",
		   info->current_version,
		   info->update_version,
		   info->update_flag,
		   info->update_process,
		   method,
		   info->update_method.storage,
		   info->update_method.location,
		   info->product,
		   info->url);

	free(info);

	return 0;
}
