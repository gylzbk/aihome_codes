#ifndef __EMMC_H__
#define __EMMC_H__

#ifdef  __cplusplus
extern "C" {
#endif

extern void *emmc_read(char *blkname, off_t offset, size_t len);
extern int emmc_write(char *blkname, off_t offset, unsigned char *data, size_t len);
extern int emmc_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);
extern int emmc_earse(char *blkname, off_t offset, size_t len);
extern size_t emmc_size_get(char *blkname);

#ifdef  __cplusplus
}
#endif

#endif //__EMMC_H__
