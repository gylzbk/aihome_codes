/**
 * @file resample_interface.h
 * @brief For the operation to resample
 * @author ydzhao <yudan.zhao@ingenic.com>
 * @version 1.0.0
 * @date 2016-04-29
 *
 * Copyright (C) 2016 Ingenic Semiconductor Co., Ltd.
 *
 * The program is not free, Ingenic without permission,
 * no one shall be arbitrarily (including but not limited
 * to: copy, to the illegal way of communication, display,
 * mirror, upload, download) use, or by unconventional
 * methods (such as: malicious intervention Ingenic data)
 * Ingenic's normal service, no one shall be arbitrarily by
 * software the program automatically get Ingenic data
 * Otherwise, Ingenic will be investigated for legal responsibility
 * according to law.
 */

/*resample_interface.h*/
#ifndef _CHANNELS_INTERFACE_H_
#define _CHANNELS_INTERFACE_H_

typedef struct af_channels_s{
int route[8][2];
int nr;
int in_chs;
int out_chs;
} af_channels_t;

#ifdef  __cplusplus
extern "C" {
#endif
/**
 * @in_chs: number of channels before change
 * @out_chs: number of channels after change
 * @brief: Design the filter and init
 * @return: On success return the af_channels_t* , and return NULL if an error occurred
 */
extern af_channels_t *mozart_channels_init(uint32_t in_chs, uint32_t out_chs);

/**
 * @brief: Deallocate memory
 */
extern void mozart_channels_uninit(af_channels_t *s);

/**
 * @inputlen: data len to change channels
 * @in_chs: number of channels before change
 * @out_chs: number of channels after change
 * @return: On succes return data len after channels change, and return 0 if an error occurred
 */
extern uint32_t mozart_channels_get_outlen(af_channels_t *s, uint32_t inputlen);
/*
 * @bytespersample: bit width in bytes
 * @is_le: little endian set 1 otherwise 0
 * @is_us: unsigned format set 1 otherwise 0
 * @inbuff: data buffer before channels change
 * @inputlen: data len in bytes before channels change
 * @outbuff: data buffer after channels change
 * @brief: channels change
 */
extern void mozart_channels(af_channels_t *s, uint32_t bytespersample,
		uint32_t is_le, uint32_t is_us, char *inbuff, uint32_t inputlen, char *outbuff);

#ifdef  __cplusplus
}
#endif

#endif /* _CHANNELS_INTERFACE_H_ */
