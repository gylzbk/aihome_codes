#include <stdio.h>
#include "cgi.h"
#include "utils_interface.h"
#include "ota_interface.h"
#include "tips_interface.h"

int main(void)
{
	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	mozart_ota_start_recovery();

	cgi_end();

	return 0;
}
