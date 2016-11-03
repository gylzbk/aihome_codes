#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "urlparser.h"

void urlp_init(url_parser *url)
{
	url->protocol = NULL;
	url->address = NULL;
	url->port = -1;
	url->file = NULL;

	return;
}

void urlp_uninit(url_parser *url)
{
	free(url->protocol);
	url->protocol = NULL;

	free(url->address);
	url->address = NULL;

	url->port = -1;

	free(url->file);
	url->file = NULL;

	return;
}

url_parser *urlp_parse(url_parser *url, const char *path)
{
	char *path_iter = NULL;
	int i = 0;
	char *sep1 = NULL;
	char *sep2 = NULL;

	char *protocols[] = {
		"http",
		"https",
		"ftp",
		"tftp",
	};

	if (!url) {
		printf("url is null, please init firstly!\n");
		return NULL;
	}

	if (!path) {
		printf("path given is null, do nothing.\n");
		return NULL;
	}
	//Now, path_iter is "http://1.2.3.4:80/release/version.ini"
	path_iter = (char *)path;

	// 1. parse protocol.
	for (i = 0; i < sizeof(protocols) / sizeof(protocols[0]); i++) {
		if(!strncasecmp(path_iter, protocols[i], strlen(protocols[i]))) {
			url->protocol = malloc(strlen(protocols[i]) + 1);
			memset(url->protocol, 0, strlen(protocols[i]) + 1);
			strcpy(url->protocol, protocols[i]);
			break;
		}
	}

	if (i == sizeof(protocols) / sizeof(protocols[0])) {
		printf("Unspoort protocol, do nothing.\n");
		return NULL;
	}

	// skip protocol domain.
	//Now, path_iter is "//1.2.3.4:80/release/version.ini"
	path_iter += strlen(url->protocol);

	// skip "://" mark.
	if (strncmp(path_iter, "://", strlen("://"))) {
		printf("Invalid path, please check!!\n");
		return NULL;
	}
	//Now, path_iter is "1.2.3.4:80/release/version.ini"
	path_iter += strlen("://");


	// 2. parse address and port.
	sep1 = strstr(path_iter, ":");
	sep2 = strstr(path_iter, "/");

	// there must be a '/' mark, else that is a wrong url!!(http://1.2.3.4)
	if (!sep2) {
		printf("Invalid path, please check!!\n");
		return NULL;
	}

	// if not found ':', there only have address, port is default: 80.(http://1.2.3.4/release/verson.ini)
	if (!sep1) {
		url->address = malloc(strlen(path_iter) - strlen(sep2) + 1);
		memset(url->address, 0, strlen(path_iter) - strlen(sep2) + 1);
		strncpy(url->address, path_iter, strlen(path_iter) - strlen(sep2));

		url->port = 80;
	} else {
		// if ':' behind '/', there only have address, port is default: 80.(http://1.2.3.4/release/ver:son.ini)
		if (sep1 > sep2) {
			url->address = malloc(strlen(path_iter) - strlen(sep2) + 1);
			memset(url->address, 0, strlen(path_iter) - strlen(sep2) + 1);
			strncpy(url->address, path_iter, strlen(path_iter) - strlen(sep2));

			url->port = 80;
		} else { // else there have address and port.(http://1.2.3.4:8723/release/ver:son.ini)
			url->address = malloc(strlen(path_iter) - strlen(sep2) + 1);
			memset(url->address, 0, strlen(path_iter) - strlen(sep2) + 1);
			strncpy(url->address, path_iter, strlen(path_iter) - strlen(sep2));

			url->port = atoi(sep1 + 1);
		}
	}

	//3. parse file.
	url->file = malloc(strlen(sep2) + 1);
	memset(url->file, 0, strlen(sep2) + 1);
	strcpy(url->file, sep2);

	return url;
}

#if 0
int main(void)
{
	const char path[] = "http://10.4.50.227/release/version.ini";
	url_parser url;

	urlp_init(&url);
	urlp_parse(&url, path);

	printf("protocol: '%s'\n", url.protocol);
	printf("address: '%s'\n", url.address);
	printf("port: '%d'\n", url.port);
	printf("file: '%s'\n", url.file);

	urlp_uninit(&url);


	return 0;
}
#endif
