#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "urlparser.h"
#include "flash.h"
#include "ini_interface.h"
#include "nvrw_interface.h"
#include "utils_interface.h"
#include "debug.h"

#define MAXLINE 8192
#define DATASIZE (64 * 1024)

#define REQUEST \
	"GET %s HTTP/1.1\r\n" \
	"Host: %s:%d\r\n\r\n"

#define CHECK_MD5 1

#if CHECK_MD5
#include "openssl/md5.h"
#endif

static bool http_is_header_entire(char *buffer)
{
	if (!buffer)
		return false;

	if (strstr(buffer, "\r\n\r\n"))
		return true;

	return false;
}

static char *encode_url(char *url)
{
	int i = 0;
	char *dst = NULL;

	if (!url)
		return NULL;

	dst = malloc(strlen(url) * 3 + 1);
	if (!dst)
		return NULL;
	dst[0] = '\0';

	for (i = 0; url[i]; i++) {
		switch (url[i]) {
		case ' ':
			strcat(dst, "%20");
			break;
		default:
			strncat(dst, url + i, 1);
			break;
		}
	}

	return dst;
}

static int http_tcp_open(url_parser url)
{
	int ret = -1;
	int sockfd = -1;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;
	char portstr[10] = {};

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* case IPv4 ONLY */
	hints.ai_socktype = SOCK_STREAM;
	sprintf(portstr, "%d", url.port);
	ret = getaddrinfo(url.address, portstr, &hints, &result);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo fail: %s\n", gai_strerror(ret));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype,
						rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(sockfd);
	}

	if (rp == NULL) { /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		return -1;
	}

	freeaddrinfo(result);

	return sockfd;
}


static int http_get(url_parser url, struct otafile_info f)
{
	char response[MAXLINE + 1] = {};
	char databuf[DATASIZE] = {};
	char *request = NULL;
	char *tmp = NULL;
	char *page_encode = NULL;
	int response_header_len = 0;
	int sockfd = -1;
	int cnt = 0;
	int cnt1 = 0;
	int n = 0;

#if CHECK_MD5
	MD5_CTX ctx;
	unsigned char md5[16] = {};
	char *md5str= NULL;
	char *data = NULL;
#endif

	struct timeval timeout;
	fd_set rfds;
	int max_fd;
	int ret = 0;

	sockfd = http_tcp_open(url);
	if (sockfd == -1)
		goto err_exit;

#if CHECK_MD5
	MD5_Init(&ctx);
#endif

	page_encode = encode_url(url.file);
	if (!page_encode)
		goto err_exit;

	request = malloc(strlen(url.address) + strlen(page_encode) + strlen(REQUEST) + 5/* for max port */ + 1);
	if (!request)
		goto err_exit;

	/* get http request content */
	sprintf(request, REQUEST, page_encode, url.address, url.port);

	free(page_encode);
	page_encode = NULL;

	/* send http request */
resend_request:
	n = send(sockfd, request, strlen(request), MSG_NOSIGNAL);
	if (n != strlen(request)) {
		if (n == -1 && errno == EINTR) { // send() interrupted by signal, try again.
			usleep(10);
			goto resend_request;
		} else {
			goto err_exit;
		}
	}
	free(request);
	request = NULL;

	/* FIXME: we think response header <= 8192 here.*/
	cnt1 = 0;
	n = 0;
	while (1) {
		n = recv(sockfd, response + cnt1, MAXLINE - cnt1, 0);
		if (n < 0) {
			goto err_exit;
		} else {
			cnt1 += n;
			if (http_is_header_entire(response)) {
				break;
			}
			if (cnt1 > MAXLINE) {
				printf("response too long..\n");
				goto err_exit;
			}
		}
	}
	tmp = strstr(response, "\r\n\r\n");
	if (!tmp) {
		printf("not found respons header end flag.\n");
		goto err_exit;
	}

	response_header_len = tmp - response + strlen("\r\n\r\n");

	/* write some valid data to dest position */
	cnt = cnt1 - response_header_len;

	if (cnt > 0) {
		flash_write(f.location, f.offset, response + response_header_len, cnt);
#if CHECK_MD5
		data = flash_read(f.location, f.offset, cnt);
		MD5_Update(&ctx, data, cnt);
		free(data);
#endif
		printf("\r    process: %#05x/%#05llx.", cnt, f.size);
		fflush(stdout);
	}

	/* recv left data and write to dest position */
	while (1) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		max_fd = sockfd;

		// wait fd readable
		timeout.tv_sec = 0;
		timeout.tv_usec = 500 * 1000;
		ret = select(max_fd + 1, &rfds, NULL, NULL, &timeout);
		if (ret < 0) { // select error
			pr_err("select for download %s error: %s, Re-try after 500ms.\n",
					 f.name, strerror(errno));
			usleep(500 * 1000);
			continue;
		} else if (ret == 0) { // select timeout
			pr_debug("waiting for download %s timeout, Re-try.\n", f.name);
			continue;
		}

		n = recv(sockfd, databuf, DATASIZE, 0);
		if (n < 0) {
			if (n == -1 && errno == EINTR) { // recv() interrupted by signal, try again.
				usleep(10);
				continue;
			} else {
				goto err_exit;
			}
		} else if (n == 0) {
			break;
		} else {
			flash_write(f.location, f.offset + cnt, databuf, n);
#if CHECK_MD5
			data = flash_read(f.location, f.offset + cnt, n);
			MD5_Update(&ctx, data, n);
			free(data);
#endif
			cnt += n;
			printf("\r    process: %#05x/%#05llx.", cnt, f.size);
			fflush(stdout);
		}
	}
	puts("\n");

#if CHECK_MD5
	MD5_Final(md5, &ctx);
	md5str= malloc(64);
	memset(md5str, 0, 64);

	for (n = 0; n < 16; ++n)
		sprintf(md5str + n * 2, "%02x", md5[n]);

	if (!strcmp(md5str, f.md5)) {
		free(md5str);
		pr_info("md5 check pass.\n");
		return 0;
	} else {
		free(md5str);
		pr_err("md5 check fail.\n");
		return -1;
	}
#endif

	return 0;

err_exit:
	if (request)
		free(request);

	return -1;
}

int mozart_updater_download_to_flash(char *file_url, struct otafile_info f)
{
	int ret = 0;
	url_parser url;

	urlp_init(&url);
	urlp_parse(&url, file_url);

#if 1
	printf("file: '%s'\n", url.file);
	printf("    protocol: %s\n", url.protocol);
	printf("    address: %s\n", url.address);
	printf("    port: %d\n", url.port);
	printf("    size: %#llx\n", f.size);
#endif

	ret = http_get(url, f);

	urlp_uninit(&url);

	return ret;
}

#if 0
int main(int argc, char **argv)
{
	int ret = -1;
	ret = mozart_updater_download_to_flash("http://10.4.50.212:8000/ota/canna_v1/update_v1.12140/uImage", "/dev/mtdblock5", 0);

	printf("ret: %d.\n", ret);

	return 0;
}
#endif

/**
 * @brief download file from update server.
 *
 * @param info [in] nvinfo that contain server address
 * @param i_file [in] file on server
 * @param o_file [in] download target file.
 *
 * @return return 0 on success, -1 otherwise.
 */
int update_remote_get(nvinfo_t *info, char *i_file, char *o_file)
{
	char *exep;
	int timeout = 30; /* Default 30s */
	int ret = 0;

	// delete file if exist.
	unlink(o_file);

	// get file
	exep = malloc(strlen((char *)info->url) + strlen((char *)info->product) + strlen(i_file) + 128);
	if (!exep) {
		pr_err("Alloc exep: %s\n", strerror(errno));
		return -1;
	}

	sprintf(exep, "wget -q -O %s -T %d '%s/%s/%s' 2>/dev/null", o_file, timeout, info->url, info->product, i_file);
	pr_debug("%s\n", exep);

	ret = mozart_system(exep);
	free(exep);

	if (ret == -1) {
		pr_err("Run wget failed\n");
		return -1;
	}

	return 0;
}
