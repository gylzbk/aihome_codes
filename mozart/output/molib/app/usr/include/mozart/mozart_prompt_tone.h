#ifndef __MOZART_PROMPT_TONE_H__
#define __MOZART_PROMPT_TONE_H__

extern int mozart_prompt_tone_play_sync(char *url, bool in_lock);
extern int __mozart_prompt_tone_play_sync(char *url);
extern int mozart_prompt_tone_key_sync(char *key, bool in_lock);
extern int __mozart_prompt_tone_key_sync(char *key);
extern int mozart_prompt_tone_key(char *key);
extern int mozart_prompt_tone_startup(void);
extern int mozart_prompt_tone_shutdown(void);

#endif /* __MOZART_PROMPT_TONE_H__ */
