/*********************************************************************
 * Copyright(C), 2016, aispeech CO., LTD.
 * Filename: mozart_log.h
 * Author: zhenquan.qiu
 * Version: V1.0.0
 * Date: 06/04 2016
 * Description:
 * Others:
 * History:
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#include "mozart_module.h"
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>

#include <stdlib.h>

#include "zlog.h"
#define AI_ZLOG_GB
#include "ai_zlog.h"
#include <stdarg.h>
#undef AI_ZLOG_GB

#include <stdio.h>
#include <string.h>

#include "curl/curl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <dirent.h>

#include <pthread.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <time.h>


#ifndef MOZART_RELEASE
#define AI_LOG_DEBUG
#endif

#ifdef AI_LOG_DEBUG
#define print_hex(arry, len) \
do{\
	int i = 0; \
	for(i=0; i<len; i++) \
 		printf("%2x ", arry[i]); \
 	printf("\n"); \
}while(0)
#define print_hex1(arry, len) \
	do{\
		int i = 0; \
		for(i=0; i<len; i++) \
			printf("%2c ", arry[i]); \
		printf("\n"); \
	}while(0)

#endif

#ifdef AI_LOG_DEBUG
#define pr_debug(fmt, args...)			\
	printf("[LOG] %s: "fmt, __func__, ##args)
#else  /* MOZART_LOG_DEBUG */
#define pr_debug(fmt, args...)			\
	do {} while (0)
#endif /* MOZART_LOG_DEBUG */

#define pr_err(fmt, args...)			\
	fprintf(stderr, "[LOG] [Error] %s: "fmt, __func__, ##args)

#define FALSE 0
#define TRUE 1
#define LOG_128  (128)
#define LOG_512 	(512)
#define LOG_1K 	(1024)


#define WAIT_HOUR (17)
#define SET_UPLOAD_HOUR (18)
//#define TIMED_UPLOAD (10*1.0)
#define TIMED_UPLOAD (60*60*6*1.0)

#define NETWORK_CARD_NAME "wlan0"
#define LOG_FILE			"aihome_"
#define LOG_FILE_TIME	"aihome.log"
#define REMOTE_FTP		"ftp://log_ftp:log123456@120.24.75.220"
//#define REMOTE_FTP		"ftp://xia:x@192.168.0.3"


static zlog_category_t *zc;
static int is_log_enable = FALSE;

#define get_nonce(nonce) \
do{ \
	struct timeval tpstart;\
	gettimeofday(&tpstart,NULL); \
	srand(tpstart.tv_usec); \
	nonce = 1+(int)(TIMED_UPLOAD*rand()/(RAND_MAX+1.0)); \
}while(0)

static bool find_log_file(char * const findfile, const char * const filename);
static int get_local_mac(char * const mac_addr);
static void size_upload(char * const local_path, const char * const remote_path);
static int connect_update_server(void);
static int ftp_upload_file(const char * const remote_path, char * const local_path, char * const rename);
static void time_upload(char * const local_path, const char * const remote_path);

const char * const debug_level[LOG_MAX+1] =
{
	"FATAL][",
	"ERROR][",
	"FAC_SET][",
	"KEY][",
	"VOLUME][",
	"CHANNEL][",
	"POWER][",
	"VR][",
	"DEBUG][",
	"MAX]["
};

int ai_log_init(void)
{
	if(zlog_init("/usr/data/zlog.conf"))
	{
		pr_err("log_init error \n");
		return -1;
	}

	zc = zlog_get_category("aihome_log_cat");
	if (!zc)
	{
		pr_err("get cat fail\n");
		zlog_fini();
		return -2;
	}

	connect_update_server();

	printf("aihome_log_init --> init successful \n");

	ai_log_set_enable(true);
	return 0;
}

void ai_log_set_enable(bool enalbe)
{
	is_log_enable = enalbe;
}

#if 0
static void log_format_string(log_type_e type, const char * const data, int len, char * const des)
{
	strncat(des, debug_level[type], strlen(debug_level[type]));
	strncat(des, data, len);
}
#endif

int ai_log_add(log_type_e type, const char *fmt, ...)
{
	if (is_log_enable == FALSE) {
		pr_err("ai_log_add --> enable is not \n");
		return -1;
	}

	if (fmt == NULL) {
		pr_err("ai_log_add --> fmt is NULL \n");
		return -1;
	}

	va_list args;
	int level_len;
	char tmp_str[LOG_1K] = {0};

	level_len = strlen(debug_level[type]);
	strncat(tmp_str, debug_level[type], strlen(debug_level[type]));

	va_start(args, fmt);
	vsprintf(tmp_str+level_len, fmt, args);
	va_end(args);

	if( *(tmp_str + strlen(tmp_str) - 1)  == '\n' )
		*(tmp_str + strlen(tmp_str) - 1) = '\0';

	switch(type)
	{
		case LOG_FATAL:
			zlog_fatal(zc, tmp_str);
		break;
		case LOG_ERROR:
			zlog_error(zc, tmp_str);
		break;
		case LOG_KEY:
		case LOG_FACTORY_SETTING:
		case LOG_VOLUME:
		case LOG_CHANNEL:
		//case LOG_ADDR:
		case LOG_DEBUG:
			zlog_info(zc, tmp_str);
		break;
		default:
			zlog_info(zc, tmp_str);
		break;
	}

	return 0;
}

static void check_dir(char * const path)
{
	int i = 0;
	int j = 0;
	int first_flag = 0;

	if(NULL == path)
	{
		pr_err("check_url --> path is NULL \n");
		return;
	}

	while('\0' != path[i++])
	{
		if('/' == path[i] && '/' == path[i+1])
		{
			if(!first_flag++)
				continue;

			j = i;
			while('\0' != path[j+1])
			{
				path[j+1] = path[j+2];
				j++;
			}
		}
	}
}

/* NOTE: if you want this example to work on Windows with libcurl as a
   DLL, you MUST also provide a read callback with CURLOPT_READFUNCTION.
   Failing to do so will give you a crash since a DLL may not use the
   variable's memory when passed in to it from an app like this. */
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  curl_off_t nread;
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  size_t retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  pr_debug("*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
  return retcode;
}

static int ftp_upload_file(const char * const remote_path, char * const local_path, char * const rename)
{
  CURL *curl;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  curl_off_t fsize;

  struct curl_slist *headerlist=NULL;

  char file_name[LOG_128]={0};
  char buf_1 [LOG_128] = "RNFR ";
  char buf_2 [LOG_128] = "RNTO ";
  char remote_url[LOG_128] = {0};

  int dir_len = strlen(remote_path);
  int file_len = strlen(local_path);
  if(file_len > LOG_128-5 || dir_len > LOG_128-strlen(REMOTE_FTP))
  {
	  pr_err("ftp_upload_file --> len is too long \n");
	  return -2;
  }

  /* get the file size of the local file */
  if(stat(local_path, &file_info))
  {
    pr_err("Couldnt open '%s': %s\n", local_path, strerror(errno));
    return -1;
  }
  fsize = (curl_off_t)file_info.st_size;

  /* get a FILE * of the same file */
  hd_src = fopen(local_path, "rb");

  if(!rename)
  {
	  strcpy(file_name, strrchr(local_path, '/')+1);
	  file_name[strlen(file_name)-6] = '\0';
	  strncat(file_name, "log", strlen("log"));
  }
  else
  {
	  memset(file_name, '\0', sizeof(file_name));
	  strncpy(file_name, rename, strlen(rename));
  }

  file_len = strlen(file_name);

  strncat(buf_1, file_name, file_len);
  strncat(buf_2, file_name, file_len);

  strncat(remote_url, REMOTE_FTP, strlen(REMOTE_FTP));
  strncat(remote_url, "/", 1);
  strncat(remote_url, remote_path, dir_len);
  strncat(remote_url, "/", 1);
  strncat(remote_url, file_name, file_len);

  check_dir(remote_url);

  /* In windows, this will init the winsock stuff */
 // curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* build a list of commands to pass to libcurl */
    headerlist = curl_slist_append(headerlist, buf_1);
    headerlist = curl_slist_append(headerlist, buf_2);

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);

    /* specify target */
    //curl_easy_setopt(curl, CURLOPT_URL, REMOTE_URL);
    curl_easy_setopt(curl, CURLOPT_URL, remote_url);

    /* pass in that last of FTP commands to run after the transfer */
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)fsize);

    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* clean up the FTP commands list */
    curl_slist_free_all (headerlist);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */

  curl_global_cleanup();
  return 0;
}

static bool find_log_file(char * const findfile, const char * const filename)
{
	DIR *tmp_dir = NULL;
	struct dirent *entry;
	bool find_log_flag = FALSE;

	if((tmp_dir = opendir("/tmp")) == NULL)
	{
		pr_err("find_log --> open tmp faild \n");
		return FALSE;
	}
	else
	{
		while((entry = readdir(tmp_dir)) != NULL)
		{
			if(!strncmp(entry->d_name, filename, strlen(filename)))
			{
				find_log_flag = TRUE;
				findfile[0] = '/'; findfile[1] = 't'; findfile[2] = 'm'; \
				findfile[3] = 'p'; findfile[4] = '/';
				strcpy(findfile+strlen("/tmp/"), entry->d_name);
				break;
			}
		}
		closedir(tmp_dir);

		return find_log_flag;
	}
}

static int get_local_mac(char * const mac_addr)
{
    int sock_mac;
    struct ifreq ifr_mac;

    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_mac == -1)
    {
        perror("create socket falise...mac/n");
        return -1;
    }

    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, NETWORK_CARD_NAME, sizeof(ifr_mac.ifr_name)-1);

    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
        printf("mac ioctl error/n");
        return -2;
    }

    sprintf(mac_addr+1,"%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);

	mac_addr[0] = '/';
	mac_addr[strlen(mac_addr)] = '/';

    close( sock_mac );

	return 0;
}

static void size_upload(char * const local_path, const char * const remote_path)
{
	if( !find_log_file(local_path, LOG_FILE) )
		return ;

	if( 0 != ftp_upload_file(remote_path, local_path, NULL) )
	{
		pr_err("size_upload --> ftp_upload_file is faild \n");
		return ;
	}

	remove(local_path);
}

bool timer_upload_flag = FALSE;
static bool time_upload_flag = FALSE;
typedef void (*timer_cb)(int signo);

void log_set_timer(int nonce, timer_cb timer_hander)
{
    struct itimerval tick;

    signal(SIGALRM, timer_hander);

    memset(&tick, 0, sizeof(tick));

    //Timeout to run first time
    tick.it_value.tv_sec = nonce;
    tick.it_value.tv_usec = 0;

    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
		printf("Set timer failed!\n");
}

void timer_callback(int signo)
{
	timer_upload_flag = TRUE;
}

void wait_time_upload(struct tm **timenow)
{
	int nonce;
	time_t now;

	time(&now);
	*timenow = localtime(&now);

	if(WAIT_HOUR == (*timenow)->tm_hour && !time_upload_flag)
	{
		time_upload_flag = TRUE;
		get_nonce(nonce);
		pr_debug("wait_time_upload --> nonce: %d \n\n", nonce);
		log_set_timer(nonce, timer_callback);
	}

	if((*timenow)->tm_hour == SET_UPLOAD_HOUR)
	//if((*timenow)->tm_min == SET_UPLOAD_HOUR)
	{
		time_upload_flag = FALSE;
	}
}

static void time_upload(char * const local_path, const char * const remote_path)
{
	struct tm *timenow;

	wait_time_upload(&timenow);

	//if(1)
	if(timer_upload_flag)
	{
		timer_upload_flag = FALSE;
		if( find_log_file(local_path, LOG_FILE_TIME) )
		{
			char rename[30] = {0};
			sprintf(rename, "aihome_%04d%02d%02d_%02d_%02d.log", \
					timenow->tm_year+1900, timenow->tm_mon+1, \
			   		timenow->tm_mday,      timenow->tm_hour, \
			   		timenow->tm_min);

			if( 0 != ftp_upload_file(remote_path, local_path, rename) )
			{
				pr_err("size_upload --> ftp_upload_file is faild \n");
				return ;
			}

			remove(local_path);
		}
	}
}

void * mozat_log_update(void * arg)
{
	sleep(20);

	char remote_path[15] = {0};
	char local_path[35] = {0};
  /* In windows, this will init the winsock stuff */
  	curl_global_init(CURL_GLOBAL_ALL);
	get_local_mac(remote_path);

	while(1)
	{
		memset(local_path, '\0', sizeof(local_path));
		size_upload(local_path, remote_path);

		memset(local_path, '\0', sizeof(local_path));
		time_upload(local_path, remote_path);

		sleep(30);
	}

	return NULL;
}

static int connect_update_server(void)
{
	pthread_t log_pthread_send;

	if (pthread_create(&log_pthread_send, NULL, mozat_log_update, NULL) != 0)
	{
		pr_err("Create log_pthread_send pthread fail! \n");
		return -1;
	}

	pthread_detach(log_pthread_send);

	return 0;
}

void ai_log_destroy(void)
{
	ai_log_set_enable(false);
	zlog_fini();
}

