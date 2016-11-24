#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cgi.h"

#define MAXLINE 1024

/* URL中特殊符号的转意
   +    URL 中+号表示空格              %2B
   空格 URL中的空格可以用+号或者编码   %20
   /    分隔目录和子目录               %2F
   ?    分隔实际的URL和参数            %3F
   %    指定特殊字符                   %25
   #    表示书签                       %23
   &    URL 中指定的参数间的分隔符     %26
   =    URL 中指定参数的值             %3D
*/

static char *encode_url(char *url, char *unicode_url)
{
    int i = 0;
    char *src = NULL;
    char *dst = unicode_url;

    if (!url || !unicode_url)
        return NULL;

    dst[0] = '\0';
    src = strrchr(url, '/') + 1;
    strncat(dst, url, src - url);

    for (i = 0; src[i]; i++) {
        switch (src[i]) {
        case '+':
            strcat(dst, "%2B");
            break;
        case ' ':
            strcat(dst, "%20");
            break;
        case '/':
            strcat(dst, "%2F");
            break;
        case '?':
            strcat(dst, "%3F");
            break;
        case '%':
            strcat(dst, "%25");
            break;
        case '#':
            strcat(dst, "%23");
            break;
        case '&':
            strcat(dst, "%26");
            break;
        case '=':
            strcat(dst, "%3D");
            break;
        default:
            strncat(dst, src+i,1);
            break;
        }
    }
    //printf("tmp: %s, url: %s, src: %s, dst: %s.\n", tmp, url, src, dst);

    return unicode_url;
}

int main(void)
{
    int i = 0;
    char cmd[MAXLINE] = {};
    char *name = NULL;
    char *url = NULL;

    char *url1 = NULL;
    char *url2 = NULL;
    char *name1 = NULL;

    cgi_init();
    cgi_process_form();
    cgi_init_headers();

    // name
    if (cgi_param("name")) {
        printf("name: %s<br>", (name = cgi_param("name")));
    } else {
        printf("download_fail because of empty name\n");
        cgi_end();
        return -1;
    }

    // url
    if (cgi_param("url")) {
        printf("url: %s<br>", (url = cgi_param("url")));
    } else {
        printf("download_fail because of empty url\n");
        cgi_end();
        return -1;
    }

    // encode url stage1
    url1 = malloc(strlen(url) * 2 + 1);
    memset(url1, 0, strlen(url) * 2 + 1);

    for (i = 0; url[i]; i++) {
        switch (url[i]) {
        case '"':
            strcat(url1, "\\\"");
            break;
        default:
            strncat(url1, url+i, 1);
            break;
        }
    }

    // encode url stage2
    url2 = malloc(strlen(url1) * 2);
    encode_url(url1, url2);

    // encode target name
    name1 = malloc(strlen(name) * 2);

    for (i = 0; name[i]; i++) {
        switch (name[i]) {
        case '"':
            strcat(name1, "\\\"");
            break;
        default:
            strncat(name1, name+i, 1);
            break;
        }
    }

    sprintf(cmd, "wget %s -O '/mnt/sdcard/music/download/%s'", url2, name1);

    puts(cmd);

    if (system(cmd))
        printf("download_fail\n");
    else
        printf("download_finish\n");

    free(url1);
    free(url2);
    free(name1);

    cgi_end();

    return 0;
}
