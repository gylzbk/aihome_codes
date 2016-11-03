#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "spinor.h"
#include "spinand.h"
#include "nand.h"
#include "emmc.h"

extern void *flash_read(char *blkname, off_t offset, ssize_t len);
extern int flash_earse(char *blkname, off_t offset, ssize_t len);
extern int flash_write(char *blkname, off_t offset, void *data, ssize_t len);
extern int flash_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);
extern size_t flash_size_get(char *blkname);

#ifdef  __cplusplus
}
#endif

#endif //__FLASH_H__
