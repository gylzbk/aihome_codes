#ifndef __NAND_H__
#define __NAND_H__

#ifdef  __cplusplus
extern "C" {
#endif

extern int nand_read(char *blkname, off_t offset, unsigned char *buf, size_t len);
extern int nand_write(char *blkname, off_t offset, unsigned char *data, size_t len);
extern int nand_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);
extern int nand_earse(char *blkname, off_t offset, size_t len);
extern size_t nand_size_get(char *blkname);

#ifdef  __cplusplus
}
#endif

#endif //__NAND_H__
