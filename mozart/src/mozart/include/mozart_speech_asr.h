#ifndef __MOZART_SPEECH_ASR_H__
#define __MOZART_SPEECH_ASR_H__

#include "vr-speech_interface.h"

extern int mozart_speech_asr_start(void);
extern int mozart_speech_asr_over(void);
extern int mozart_speech_asr_startup(int wakeup_mode_mark);
extern int mozart_speech_asr_shutdown(void);


#endif /* __MOZART_SPEECH_ASR_H__ */
