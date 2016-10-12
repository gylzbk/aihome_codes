#ifndef _BASELIB_H_
#define _BASELIB_H_

#include "fop.h"
#include "json_op.h"
#include "music_list.h"

int music_list_print(music_obj *m);
int low_output_cb(int arg, char *s, int size);
int high_output_cb(void *context, char *a, char *b, char *c);
int cur_output_cb(void *context, char *a, char *b, char *c);
int low_input_cb(int arg, char *s, int size);
int high_node_get(struct op *o, music_obj *m);
int machine_close(struct op *o, music_obj *m);
int music_list_cur_prev_print(music_obj *m);
int music_list_cur_next_print(music_obj *m);
int machine_open(struct op *o);
int music_restart(music_obj *m_obj);
#endif
