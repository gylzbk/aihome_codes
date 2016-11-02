#ifndef _HTTP_H_
#define _HTTP_H_

#include "nvrw_interface.h"

extern int mozart_updater_download_to_flash(char *file_url, struct otafile_info f);
extern int update_remote_get(nvinfo_t *info, char *i_file, char *o_file);

#endif

