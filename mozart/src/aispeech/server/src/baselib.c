#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "baselib.h"
#define debug 0

int music_list_current_position_get(struct op *o)
{
	music_obj *m = (music_obj *)op_context_get(o);
	int retvalue = 1;
	music_info *tmp;
	if ((o == NULL) || (m == NULL)) {
		retvalue = -1;
		print("error\n");
		goto end;
	}
	/*get music list current node*/
	retvalue = op_high_output(o, 10000);
	if (retvalue == -1) {
		print("error\n");
		goto end;
	}

	/*for music list current point move to beginning*/
	tmp = music_cur_get(m);
	if (tmp != NULL) {
		while (1) {
			print("[title:artist:url] [%s : %s : %s]\n",
			tmp->title, tmp->artist, tmp->url);
			if (!strncmp(m->cur_tmp->title, tmp->title,
					strlen(tmp->title)) &&
			    !strncmp(m->cur_tmp->artist, tmp->artist,
			    		strlen(tmp->artist)) &&
			    (!strncmp(m->cur_tmp->url, tmp->url,
			    		strlen(tmp->url)))) {
				/*XXX set current music list position*/
				print("found the current node\n");
				music_cur_set(m, tmp);
				free(m->cur_tmp->title);
				free(m->cur_tmp->artist);
				free(m->cur_tmp->url);
				break;
				} else {
			}

			tmp = music_prev_get(m);
			if (tmp == NULL) {
				print("no found the current node\n");
				break;
			}
		}
	} else {
		print("no node\n");
		goto end;
	}
end:
	return retvalue;
}

int high_node_get(struct op *o, music_obj *m)
{
	int retvalue = 1;
	int i = 0;
	music_info *tmp;
	if ((o == NULL) || (m == NULL)) {
		retvalue = -1;
		print("error\n");
		goto end;
	}
	/*for music list current point move to beginning*/
	tmp = music_cur_get(m);
	if (tmp != NULL) {
		print("insert current node to file:\n");
		print("[title:artist:url] [%s : %s : %s]\n",
		tmp->title, tmp->artist, tmp->url);

		/*XXX: magic nubmer is no good, wrap cjson format*/
		op_high_input(10000, o, tmp->title, tmp->artist, tmp->url);
	} else {
		print("no node\n");
		goto end;
	}
	
	while (1) {
		tmp = music_prev_get(m);
		if (tmp == NULL)
			break;
	}

	print("insert node to file:\n");
	/*insert first music list node*/
	tmp = music_cur_get(m);
	if (tmp != NULL) {
		print("[title:artist:url] [%s : %s : %s]\n",
			tmp->title, tmp->artist, tmp->url);

		/*wrap cjson format*/
		op_high_input(i, o, tmp->title, tmp->artist, tmp->url);
		i++;
	} else {
		print("no node\n");
		goto end;
	}
	/*loop get next music list node*/
	while (1) {
		tmp = music_next_get(m);
		if (tmp == NULL) {
			break;
		} else {
			print("[title:artist:url] [%s : %s : %s]\n",
				tmp->title, tmp->artist, tmp->url);

			/*wrap cjson format*/
			op_high_input(i, o, tmp->title, tmp->artist, tmp->url);
			i++;
		}
	}
end:
	return retvalue;
}

int machine_close(struct op *o, music_obj *m)
{
	if ((o == NULL) || (m == NULL)) {
		print("error\n");
		return 0;
	}
	int retvalue = 1;
	retvalue = high_node_get(o, m);
	if (retvalue == -1) {
		goto end;
	}
	op_low_output(o);
end:
	return retvalue;
}

int music_list_cur_prev_print(music_obj *m)
{
	int retvalue = 1;
	if (m == NULL) {
		retvalue = -1;
		print("error\n");
		goto end;
	}

	music_info *tmp = music_cur_get(m);
	if (tmp != NULL) {
		print("[title:artist:url] [%s : %s : %s]\n",
			tmp->title, tmp->artist, tmp->url);
	} else {
		retvalue = -1;
		print("error\n");
		goto end;
	}
	while (1) {
		tmp = music_prev_get(m);
		if (tmp == NULL) {
			print("node end\n");
			break;
		} else {
			print("[title:artist:url] [%s : %s : %s]\n",
				tmp->title, tmp->artist, tmp->url);
		}
	}
end:
	return retvalue;
}

int music_list_cur_next_print(music_obj *m)
{
	int retvalue = 1;
	if (m == NULL) {
		retvalue = -1;
		print("error\n");
		goto end;
	}

	music_info *tmp = music_cur_get(m);
	if (tmp != NULL) {
		print("[title:artist:url] [%s : %s : %s]\n",
			tmp->title, tmp->artist, tmp->url);
	} else {
		retvalue = -1;
		print("error\n");
		goto end;
	}
	while (1) {
		tmp = music_next_get(m);
		if (tmp == NULL) {
			print("node end\n");
			break;
		} else {
			print("[title:artist:url] [%s : %s : %s]\n",
				tmp->title, tmp->artist, tmp->url);
		}
	}
end:
	return retvalue;
}

int machine_open(struct op *o)
{
	int retvalue = 1;

	if (o == NULL) {
		print("error\n");
		goto end;
	}

	/*read low config message to op buf*/
	retvalue = op_low_input(o);
	if (retvalue == -1) {
		goto end;
	}

	/*output op buf message to high layer*/
	int i;
	for (i = 0; i < 20; i++) {
		op_high_output(o, i);
	}
	music_list_current_position_get(o);
#if 0
	/*notice this will change current music position*/
	music_obj *m = op_context_get(o);
	music_list_cur_prev_print(m);
#endif

end:
	return retvalue;
}

int music_list_print(music_obj *m)
{
	if (m == NULL)
		goto end;

	/*for music list current point move to beginning*/
	music_info *tmp;
	while (1) {
		tmp = music_prev_get(m);
		if (tmp == NULL)
			break;
	}

	tmp = music_cur_get(m);
	if (tmp != NULL) {
		print("[title:artist:url] [%s : %s : %s]\n",
			tmp->title, tmp->artist, tmp->url);
	} else {
		print("no node\n");
		goto end;
	}

	/*loop get next music list node*/
	while (1) {
		tmp = music_next_get(m);
		if (tmp == NULL) {
			break;
		} else {
			print("[title:artist:url] [%s : %s : %s]\n",
				tmp->title, tmp->artist, tmp->url);
		}
	}
end:
	return 0;
}

/*user callback*/
int low_output_cb(int arg, char *s, int size)
{
	int retvalue = 1;
	if (arg <= 0) {
		print("error\n");
		retvalue = -1;
		goto end;
	}

	if (s == NULL) {
		retvalue = -1;
		goto end;
	}
	print("%s [%d]\n", s, size);
	file_write(arg, s, size);
end:
	return retvalue;
}

/*user callback*/
int high_output_cb(void *context, char *a, char *b, char *c)
{
	music_obj *m = (music_obj *)context;
	int retvalue = 1;

	if ((m == NULL) || (a == NULL) || (b == NULL) || (c == NULL)) {
		print("error\n");
		retvalue = -1;
		goto end;
	}
#if debug
	print("[%s] [%s] [%s]\n", a, b, c);
#endif
	music_info *tmp;
	music_info_alloc(&tmp, a, b, c);
	music_list_insert(m, tmp);
end:
	return retvalue;
}

/*user callback*/
int cur_output_cb(void *context, char *a, char *b, char *c)
{
	music_obj *m = (music_obj *)context;
	int retvalue = 1;

	if ((m == NULL) || (a == NULL) || (b == NULL) || (c == NULL)) {
		print("error\n");
		retvalue = -1;
		goto end;
	}

	print("[%s] [%s] [%s]\n", a, b, c);
	/*XXX: recycle memeory*/
	m->cur_tmp->title = strdup(a);
	m->cur_tmp->artist = strdup(b);
	m->cur_tmp->url = strdup(c);
end:
	return retvalue;
}

/*user callback*/
int low_input_cb(int arg, char *s, int size)
{
	int retvalue = 1;

	if (arg <= 0) {
		print("error\n");
		retvalue = -1;
		goto end;
	}

	size = file_read(arg, s, size);
	if (size == -1) {
		retvalue = -1;
		goto end;
	}
end:
	return retvalue;
}

/*clear music list and restart*/
int music_restart(music_obj *m_obj)
{
	music_list_destroy(&m_obj);
	music_list_alloc(&m_obj, 20);
	return 0;
}
