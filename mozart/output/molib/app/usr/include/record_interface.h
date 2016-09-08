#ifndef __record_INTERFACE_HEAD__
#define __record_INTERFACE_HEAD__

#ifdef __cplusplus
extern "C"{
#endif

#include "alsa/asoundlib.h"


/**
 * @brief audio driver
 */
typedef enum audio_driver {
        /**
         * @brief oss audio driver
         */
        OSS = 1,
        /**
         * @brief alsa audio driver
         */
        ALSA = 2,
} audio_driver;

/**
 * @brief oss audio driver device info
 */
typedef struct oss_dev {
	int dsp;
	int mixer;
} oss_dev_t;


/**
 * @brief alsa audio driver device info
 */
typedef struct alsa_dev {
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
        snd_pcm_uframes_t nframes;
} alsa_dev_t;


/**
 * @brief record formats.
 */
typedef struct record_param {
        /**
         * @brief sampling width: 8, 16, 24, 32, force little endian.
         */
        int bits;
        /**
         * @brief sampling rate: 8000, 16000, 24000.
         */
        unsigned int rates;
        /**
         * @brief record channel numbers: 1, 2, 3, 4.
         */
        unsigned int channels;
        /**
         * @brief record volume: 0-100.
         */
        int volume;
} record_param;

/**
 * @brief record handler.
 */
typedef struct mic_record {
	/**
	 * @brief audio driver type
	 */
        audio_driver driver;
	/**
	 * @brief audio driver info
	 */
        union {
                alsa_dev_t alsa;
                oss_dev_t oss;
        } dev;
	/**
	 * @brief record format
	 */
        record_param param;
	int detect_time;
} mic_record;

/**
 * @brief aec record handler.
 */
typedef struct aec_record {
	/**
	 * @brief time deltae between dmic and loopback
	 */
	int time_delta_ms;
	/**
	 * @brief loopback record format
	 */
        record_param loopback_param;
	/**
	 * @brief loopback record handler
	 */
	struct mic_record *loopback_record;
	/**
	 * @brief dmic record format
	 */
        record_param dmic_param;
	/**
	 * @brief dmic record handler
	 */
	struct mic_record *dmic_record;
} aec_record;

/**
 * @brief init record device, get a mic record.
 *
 * @param record_param record device format.
 *
 * @return return record device handler on success, otherwise return NULL.
 */
extern mic_record *mozart_soundcard_init(record_param);

/**
 * @brief destory the record device.
 *
 * @param record_info the handler of record device.
 *
 * @return return 0 on success, otherwise return -1;
 */
extern int mozart_soundcard_uninit(mic_record *record_info);

/**
 * @brief record some data
 *
 * @param record_info the handler of record device
 * @param databuf dest buf.
 * @param len record length(unit: bytes).
 *
 * @return return the real length of recording data, otherwise return -1.
 */
extern unsigned long mozart_record(mic_record *record_info, void *databuf, unsigned long len);

extern aec_record *mozart_soundcard_aec_init(record_param loopback_param, record_param dmic_param);
extern int mozart_soundcard_aec_uninit(aec_record *record);
extern int mozart_soundcard_aec_enable(aec_record *record, int time_delta_ms);
extern int mozart_soundcard_aec_disable(aec_record *record);
extern unsigned long  mozart_soundcard_aec_record(aec_record *record, char *loopback_buf,
						  char *dmic_buf, unsigned long len);

extern int mozart_soundcard_dmic_gain_auto_control(mic_record *record, void *buff, int size);

#ifdef __cplusplus
}
#endif

#endif /* __record_INTERFACE_HEAD__ */
