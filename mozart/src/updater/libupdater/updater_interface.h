#ifndef _UPDATER_H_
#define _UPDATER_H_

#include "nvrw_interface.h"

extern int mozart_updater_nvrw(struct nv_info *info);
extern struct nv_info *mozart_updater_chkver(void);
extern int mozart_updater_download(struct nv_info *info);
extern int mozart_updater_update(struct nv_info *info);

#endif /* _UPDATER_H_ */
