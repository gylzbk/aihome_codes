#ifndef __SPINOR_H__
#define __SPINOR_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern void *spinor_read(char *blkname, off_t offset, size_t len);
extern int spinor_write(char *blkname, off_t offset, unsigned char *data, size_t len);
extern int spinor_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);
extern int spinor_earse(char *blkname, off_t offset, size_t len);
extern size_t spinor_size_get(char *blkname);

#ifdef  __cplusplus
}
#endif

#endif //__SPINOR_H__
