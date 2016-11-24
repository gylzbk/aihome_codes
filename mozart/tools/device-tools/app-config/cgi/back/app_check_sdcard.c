#include <stdlib.h>
#include <stdio.h>

#include "cgi.h"
#include "utils_interface.h"

int main(void)
{
        cgi_init();
        cgi_process_form();
        cgi_init_headers();

        if(!mozart_path_is_mount("/mnt/sdcard")){
                printf("download_sdout");
                cgi_end();
                return 0;
        }else{
                sdinfo sdcardinfo = mozart_get_sdcard_info("/mnt/sdcard/");
                if(sdcardinfo.availableSize < 10){
                        printf("download_sdshort");
                        cgi_end();
                        return 0;
                }
        }

        return 0;
}
