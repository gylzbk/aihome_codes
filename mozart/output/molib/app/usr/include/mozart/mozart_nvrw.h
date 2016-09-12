/*
	mozart_nvrw.h
 */
#ifndef __MOZART_NVRW_H__
#define __MOZART_NVRW_H__

#include <stdint.h>

struct nv_info {
	uint32_t	update_status;
	uint32_t	update_result;

	float		current_version;
	float		update_version;
};

extern int nvrw_set_update_flag(float version);

extern int nvrw_get_update_info(struct nv_info *info);

extern int nvrw_clear_update_flag(void);

extern int nvrw_clear_update_result(void);

#endif /* __MOZART_NVRW_H__ */
