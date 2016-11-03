#ifndef _BLOCK_H
#define _BLOCK_H

/**
 * @brief get blk device size.
 *
 * @param blkname blk device
 *
 * @return return size on success, -1 otherwise.
 */
extern long long block_size_get(char *blkname);

/**
 * @brief read some data from blk device.
 *
 * @param blkname [in] block device.
 * @param offset [in] start read postion.
 * @param size [in] read size.
 *
 * @return return data on success(MAST free() after used done), NULL otherwise.
 */
extern void *block_read(char *blkname, off_t offset, ssize_t size);

/**
 * @brief write some data to blk device.
 *
 * @param blkname [in] block device.
 * @param offset [in] start read postion.
 * @param data [in] data to be write.
 * @param size [in] read size.
 *
 * @return return 0 on success, -1 otherwise.
 */
extern int block_write(char *blkname, off_t offset, void *data, size_t size);

/**
 * @brief write file to blk device.
 *
 * @param blkname [in] block device.
 * @param file
 * @param seek [in] seek bytes at start of blk
 * @param skip [in] skip bytes at start of file
 * @param size [in] write size bytes of file, Not implment now.
 *
 * @return return 0 on success, -1 otherwise.
 */
extern int block_write_file(char *blkname, char *file, off_t seek, off_t skip, size_t size);

#endif //_BLOCK_H
