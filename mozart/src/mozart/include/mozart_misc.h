/*
	mozart_misc.h
 */
#ifndef __mozart_misc_h__
#define __mozart_misc_h__

extern int mozart_ext_storage_enough_free(const char *path, int reserve_size);

extern int mozart_ext_storage_file_clear(const char *dir);

#endif /* __mozart_misc_h__ */
