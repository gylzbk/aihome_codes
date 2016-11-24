#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

#include "cgi.h"
#include "cJSON.h"
#include "alarm_interface.h"

/////////////////////////////////////////

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();
	
	get_alarm_mode_to_app();
	
	cgi_end();

	return 0;
}


