#ifndef __CLOUD_PARAM_H__ 
#define __CLOUD_PARAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* appKey,secretKey must be terminated by '\0'.
 * return value:On success, a pointer is returned, must be freed by caller.  On error, NULL is returned.
 * */
char *cloud_param_build(char *appKey, char *secretKey);

#ifdef __cplusplus
}
#endif
#endif
