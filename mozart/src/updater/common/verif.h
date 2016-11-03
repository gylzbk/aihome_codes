#ifndef __VERIF_H__
#define __VERIF_H__

#ifdef  __cplusplus
extern "C" {
#endif


/**
 * @brief readback from spi, and compare it's md5 with Inimage's md5.
 *
 * @param Inimage_path [in] Inimage file path
 * @param offset [in] start postion of Inimage burned to spi
 *
 * @return return 0 if same, -1 otherwise.
 */
extern int check_Inimage_on_spi(char *Inimage_path, unsigned int offset);


/**
 * @brief get md5 of file
 *
 * @param file [in] ...
 *
 * @return return string format of md5sum on success, NULL otherwise.
 */
extern char *md5sum(char *file);

/**
 * @brief verif md5 writen to flash
 *
 * @param file compare file with flash data.
 * @param offset file write to flash offset(Bytes)
 *
 * @return return 0 if same, -1 otherwise.
 */
extern int verif_file_on_flash(char *file, unsigned int offset);

#ifdef  __cplusplus
}
#endif

#endif //__VERIF_H__
