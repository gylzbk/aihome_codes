/*
	nv_wr.h
 */

#ifndef __NV_WR_H__
#define __NV_WR_H__

#define NV_NUMBERS	2

struct status_flag_bits {
	unsigned int ota_start:1;		/* start */
	unsigned int load_new_fs:1;		/* new_flag */
	unsigned int update_fs_finish:1;	/* update_kernel_finish */
	unsigned int user_fs_finish:1;		/* update_fs_finish */
};

struct nv_area_wr {
	unsigned int	write_start;
	char		url_1[512];
	char		url_2[512];
	union {
		unsigned int current_version_i;
		float current_version_f;
	};
	float		update_version;
	union {
		unsigned int update_status;
		struct status_flag_bits sfb;
	};
	unsigned int	update_result;
	unsigned int	block_current_count;
	unsigned int	kernel_size;
	unsigned int	update_size;
	unsigned char	resever[32 * 1024 - 1024 - 4 * 10];
	unsigned int	write_count;
	unsigned int	write_end;
};

#endif /* __NV_WR_H__ */
