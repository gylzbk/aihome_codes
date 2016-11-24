#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "cgi.h"
#include "linklist_interface.h"
#include "alarm_interface.h"

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	int time = mozart_time_get_clock_timestamp();
	printf("%d", time);

	cgi_end();

	return 0;
}
