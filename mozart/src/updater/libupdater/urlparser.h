#ifndef __URL_PARSER_H_
#define __URL_PARSER_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct url_parser {
	char *protocol;
	char *address;
	unsigned int port;
	char *file;
} url_parser;

/**
 * @brief init a url
 *
 * @param url [out] url to be initted
 */
extern void urlp_init(url_parser *url);

/**
 * @brief parse a url address to url_parser struct.
 *
 * @param url [out] url_parser struct var.
 * @param path [in] parse path to url element.
 *
 * @return return a url on success, NULL otherwise.
 */
extern url_parser *urlp_parse(url_parser *url, const char *path);

/**
 * @brief uninit a url.
 *
 * @param url [out] url to be uninitted.
 */
extern void urlp_uninit(url_parser *url);

#ifdef  __cplusplus
}
#endif

#endif // __URL_PARSER_H_
