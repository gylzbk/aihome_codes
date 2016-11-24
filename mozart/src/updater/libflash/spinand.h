#ifndef __SPINAND_H__
#define __SPINAND_H__

#ifdef  __cplusplus
extern "C" {
#endif

extern void *spinand_read(char *blkname, off_t offset, size_t len);
extern int spinand_write(char *blkname, off_t offset, unsigned char *data, size_t len);
extern int spinand_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);
extern int spinand_earse(char *blkname, off_t offset, size_t len);
extern size_t spinand_size_get(char *blkname);

#ifdef  __cplusplus
}
#endif

#endif //__SPINAND_H__
