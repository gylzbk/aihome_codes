#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/soundcard.h>
//#include "ini_interface.h"
//#include "utils_interface.h"
#include "vr-speech_interface.h"
#include "aiengine_app.h"
#include "echo_wakeup.h"
//#include "volume_interface.h"
#include "ai_sound_dev.h"
/* #define AEC_FILE_DEBUG */
#define AEC_AKM4951

int fddmic[2] = {-1};
int fdplay[2] = {-1};
int fd_dsp_rp = -1;
int fd_dsp_rd = -1;
extern int fdr;
extern int fdp;

int pipe_init(void)
{
	if (pipe(fddmic) < 0) {
		perror("pipe");
		goto error_quit;
	}
	if (pipe(fdplay) < 0) {
		perror("pipe");
		goto error_quit;
	}

	return 0;
error_quit:
	pipe_close();
	return -1;

}

void sound_device_release(void)
{
    if(fd_dsp_rp > 0)
    {
        close(fd_dsp_rp);
        fd_dsp_rp = -1;
    }

    if(fd_dsp_rd > 0)
    {
        close(fd_dsp_rd);
        fd_dsp_rd = -1;
    }
}


void pipe_close(void)
{
	if (fddmic[0] > 0) {
		close(fddmic[0]);
		fddmic[0] = -1;
	}
	if (fddmic[1] > 0) {
		close(fddmic[1]);
		fddmic[1] = -1;
	}
	if (fdplay[0] > 0) {
		close(fdplay[0]);
		fdplay[0] = -1;
	}
	if (fdplay[1] > 0) {
		close(fdplay[1]);
		fdplay[1] = -1;
	}
}

int sound_device_init_near(int val)
{
	int status;
	int fd_mixer = open("/dev/mixer3", O_WRONLY);
	if (fd_mixer < 0) {
		printf("open /dev/mixer failed\n");
		perror("open");
		goto error_quit;
	}

	status = ioctl(fd_mixer, SOUND_MIXER_WRITE_MIC, &val);
	if (status < 0) {
		printf("ioctl select loopback device failed\n");
		goto error_quit;
	}

	fd_dsp_rd = open("/dev/dsp3", O_RDONLY);
	if (fd_dsp_rd < 0) {
		perror("open /dev/dsp3 -read failed");
		goto error_quit;
	}

	val = BIT;
	status = ioctl(fd_dsp_rd, SNDCTL_DSP_SETFMT, &val);
	if (status == -1) {
		perror("SOUND_PCM_WRITE_BITS ioctl failed");
		goto error_quit;
	}

	val = CHANEL;
	status = ioctl(fd_dsp_rd, SNDCTL_DSP_CHANNELS, &val);
	if (status == -1) {
		perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
		goto error_quit;
	}

	val = RATE;
	status = ioctl(fd_dsp_rd, SNDCTL_DSP_SPEED, &val);
	if (status == -1) {
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");
		goto error_quit;
	} else
		goto finish_quit;

error_quit:
	if (fd_mixer > 0) {
		close(fd_mixer);
		fd_mixer = -1;
	}
	if (fd_dsp_rd > 0) {
		close(fd_dsp_rd);
		fd_dsp_rd = -1;
	}
	return -1;
finish_quit:
	close(fd_mixer);
	fd_mixer = -1;
	return 0;
}

int sound_device_init_far(void)
{
	int val = 0;
	int status;
	fd_dsp_rp = -1;

	fd_dsp_rp = open("/dev/dsp", O_RDONLY);
	if (fd_dsp_rp < 0) {
		perror("open /dev/dsp -read failed");
		goto error_quit;
	}

	val = BIT;
	status = ioctl(fd_dsp_rp, SNDCTL_DSP_SETFMT, &val);
	if (status < 0) {
		perror("SOUND_PCM_WRITE_BITS ioctl failed");
		goto error_quit;
	}

#ifdef AEC_AKM4951
	val = 2;
#else
	val = CHANEL;
#endif
	status = ioctl(fd_dsp_rp, SNDCTL_DSP_CHANNELS, &val);
	if (status < 0) {
		perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
		goto error_quit;
	}

#ifdef AEC_AKM4951
	val = 48000;
#else
	val = RATE;
#endif
	status = ioctl(fd_dsp_rp, SNDCTL_DSP_SPEED, &val);
	if (status < 0) {
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");
		goto error_quit;
	} else
		goto finish_quit;

error_quit:
	if (fd_dsp_rp > 0)
		close(fd_dsp_rp);
	return -1;
finish_quit:
	return 0;
}

int sound_device_init(int vol)
{
	int ret = sound_device_init_near(vol);
	if (ret == -1) {
		printf("sound_device_init_near failed\n");
		return -1;
	}

	ret = sound_device_init_far();
	if (ret == -1) {
		printf("sound_device_init_far failed\n");
		return -1;
	}

	return 0;
}

int sound_aec_enable(void)
{
	int aec_able = 1;
	int ret = 0;
	char buftmp[AEC_SIZE] = {0};
	ret = ioctl(fd_dsp_rp, SNDCTL_DSP_AEC_ABLE, &aec_able);
	if (ret != 0) {
		printf("ioctl set aec enable failed\n");
		perror("ioctl");
		goto error_quit;
	}
	ret = read(fd_dsp_rp, buftmp, AEC_SIZE);
	if (ret != 0) {
		printf("first read dsp0 but not ready\n");
		perror("read");
		goto error_quit;
	}
	ret = read(fd_dsp_rd, buftmp, AEC_SIZE);
	if (ret != 0) {
		printf("first read dsp3 but not ready\n");
		perror("read");
		goto error_quit;
	}
	ret = 0;
	ret = ioctl(fd_dsp_rp, SNDCTL_DSP_AEC_START, &ret);
	if (ret != 0) {
		printf("DSP_AEC_START is not ready\n");
		perror("ioctl");
		goto error_quit;
	} else
		goto finish_quit;

error_quit:
	return -1;
finish_quit:
	return 0;
}

int sound_aec_disable(void)
{
	int ret = 0;
	int aec_able = 0;
	ret = ioctl(fd_dsp_rp, SNDCTL_DSP_AEC_ABLE, &aec_able);
	if (ret != 0)
		return -1;
	else
		return 0;
}

void *dmic_read(void *args)
{
//	is_dmic_running = true;
	int status = 0;
	char bufdmic[AEC_SIZE] = {0};
	DEBUG("-------------------> Start dmic_read\n");
	while (AEC_WAKEUP_TID1_EXIT!=ai_aec_flag.state) {
		status = read(fd_dsp_rd, bufdmic, AEC_SIZE);
		if (status != AEC_SIZE) {
			ai_aec_flag.error = true;
			goto result;
		}

		status = write(fddmic[1], bufdmic, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == -1) {
				goto result;
			} else {
				printf("write fddmic[1] error not AEC_SIZE\n");
				ai_aec_flag.error = true;
				goto result;
			}
		}
	}
result:
	if (fddmic[1] >= 0) {
		close(fddmic[1]);
		fddmic[1] = -1;
	}
	DEBUG("------------------> Stop dmic_read\n");
//	is_dmic_running = false;
	pthread_exit(&status);
}

void *loopback_read(void *args)
{
//	is_loopback_running = true;
	int  status = 0;
	char bufloop_tmp[AEC_SIZE * 6] = {0};
	short bufloop[AEC_SIZE / 2] = {0};
	/*
	 * then read total 47 ms audio align data
	 * which caused by hardware for aec align
	 */
	/* read 30 ms data */
	status = read(fd_dsp_rp, bufloop_tmp, 30 * MS * 6);
	if (status != 30 * MS * 6) {
		ai_aec_flag.error = true;
		goto result;
	}
	/* read 17 ms data */
	status = read(fd_dsp_rp, bufloop_tmp, 17 * MS * 6);
	if (status != 17 * MS * 6) {
		ai_aec_flag.error = true;
		goto result;
	}
	DEBUG("--------------------> Start loopback_read\n");
	while (AEC_WAKEUP_TID2_EXIT != ai_aec_flag.state) {

		status = read(fd_dsp_rp, bufloop_tmp, AEC_SIZE * 6);
		if (status != AEC_SIZE * 6) {
			ai_aec_flag.error = true;
			goto result;
		}

#ifdef AEC_AKM4951
		{
			int i;
			short *tmp;

			for (i = 0; i < AEC_SIZE / 2; i++) {
				tmp = (short *)(bufloop_tmp + 8 + i * 12);
				bufloop[i] = *tmp / 2 + *(tmp + 1) / 2;
			}

		}
#endif
		status = write(fdplay[1], bufloop, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == -1) {
				goto result;
			} else {
				printf("write fdpla[1] err not AEC_SIZE\n");
				perror("write");
				ai_aec_flag.error = true;
				goto result;
			}
		}
	}
result:
	if (fdplay[1] >= 0) {
		close(fdplay[1]);
		fdplay[1] = -1;
	}
	DEBUG("-------------------> Stop loopback_read\n");
//	is_loopback_running = false;
	pthread_exit(&status);
}

void *aec_handle(void *args)
{
	int status = 0;
//	is_aec_read_running = true;
    echo_wakeup_t *ew = (echo_wakeup_t *)args;
	char bufr[AEC_SIZE] = {0};
	char bufp[AEC_SIZE] = {0};
	DEBUG("-----------------------------> Start aec_handle\n");
	while (AEC_WAKEUP != ai_aec_flag.state && !ai_aec_flag.error && !ai_aec_flag.set_end) {
		status = read(fddmic[0], bufr, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == 0) {
				goto result;
			} else {
				ai_aec_flag.error = true;
				goto result;
			}
		}
		status = read(fdplay[0], bufp, AEC_SIZE);
		if (status != AEC_SIZE) {
			if (status == 0) {
				goto result;
			} else {
				printf("read fdplay[0] err not AEC_SIZE\n");
				ai_aec_flag.error = true;
				goto result;
			}
		}
		#ifdef AEC_FILE_DEBUG
				write(fdr, bufr, AEC_SIZE);
				write(fdp, bufp, AEC_SIZE);
		#endif	//*/
		echo_wakeup_process(ew, bufr, bufp, AEC_SIZE);
	}

result:
	if (fddmic[0] >= 0) {
		close(fddmic[0]);
		fddmic[0] = -1;
	}
	if (fdplay[0] >= 0) {
		close(fdplay[0]);
		fdplay[0] = -1;
	}
	DEBUG("----------------------> Stop aec_handle \n");
//	is_aec_read_running = false;
	pthread_exit(&status);
}
