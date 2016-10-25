#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cgi.h"

int main(void)
{
    int i = 0;
    char *name = NULL;
    char *path = NULL;

    cgi_init();
    cgi_process_form();
    cgi_init_headers();

    // name
    if (cgi_param("name")) {
        printf("name: %s<br>", (name = cgi_param("name")));
    } else {
        printf("filename=null\n");
        cgi_end();
        return -1;
    }

    path = malloc(strlen("/mnt/sdcard/music/download/") + strlen(name) + 1);

    sprintf(path, "/mnt/sdcard/music/download/%s", name);

    if (access(path, 0))
        printf("file not found\n");
    else
        printf("file found\n");

    cgi_end();

    return 0;
}
