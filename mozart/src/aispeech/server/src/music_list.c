#include "music_list.h"

int music_list_alloc(music_obj **obj, int max)
{
	*obj = malloc(sizeof(music_obj));
	int retvalue = 0;

	if ((*obj == NULL) || (max == 0)) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
		goto error;
	}

	_list_init(&(*obj)->head.list);
	(*obj)->head.title = NULL;
	(*obj)->head.artist = NULL;
	(*obj)->head.url = NULL;
	(*obj)->max = max;
	(*obj)->cur_num = 0;
	(*obj)->cur_music = NULL;
error:	
	return retvalue;
}

music_info *music_cur_get(music_obj *obj)
{
	if (obj->cur_music == NULL)
		return NULL;

	music_info *tmp = list_entry(&obj->cur_music->list, music_info, list);
	return (tmp)? tmp: NULL;
}

music_info *music_next_get(music_obj *obj)
{
	if (obj == NULL)
		return NULL;
	if (obj->cur_music == NULL)
		return NULL;
	music_info *next = list_entry(obj->cur_music->list.next,
					music_info,
					list);

	music_info *cur = music_cur_get(obj);
	if (next == cur) {
		return NULL;
	} else {
		obj->cur_music = next;
		return next;
	}
}

music_info *music_prev_get(music_obj *obj)
{
	if (obj->cur_music == NULL) {
		return NULL;
	}
	music_info *prev = list_entry(obj->cur_music->list.prev,
					music_info,
					list);
	music_info *cur = music_cur_get(obj);
	if (prev->url == NULL) {
		return NULL;
	}
	
	if (prev == cur) {
		return NULL;
	} else {
		obj->cur_music = prev;
		return prev;
	}
}

int music_list_delete(music_obj *obj, music_info **info)
{
	int retvalue = 0;
	if (*info == NULL) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
		goto error;
	}
	if ((*info)->url == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", (*info)->url);
	}

	if ((*info)->artist == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", (*info)->artist);
	}

	if ((*info)->title == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", (*info)->title);
	}

	LIST *tmp = &obj->head.list;
	music_info *m;
	while (!is_list_last(tmp)) {
		m = list_entry(tmp, music_info, list);
		if (m->url != NULL) {
			if (0 == strncmp(m->url,
					(*info)->url,
					strlen((*info)->url))) {
				if ((*info)->artist) {
					if (0 == strncmp(m->artist,
							(*info)->artist,
							strlen((*info)->artist))) {

						goto end;
					}
				}
			}
		}
		tmp = tmp->next;
	}
	m = list_entry(tmp, music_info, list);
	if (m->url != NULL) {
		if (0 == strncmp(m->url, (*info)->url, strlen((*info)->url))) {
			if ((*info)->artist) {
				if (0 == strncmp(m->artist,
						(*info)->artist,
						strlen((*info)->artist))) {
						goto end;
				}
			}
		}
	}
	
	retvalue = -1;
	return retvalue;
end:
	_list_delete(&(*info)->list);
	free((*info)->title);
	(*info)->title = NULL;
	free((*info)->artist);
	(*info)->artist = NULL;
	free((*info)->url);
	(*info)->url = NULL;
	free((*info));
	(*info) = NULL;
error:
	return retvalue;
}

int music_list_insert_head(music_obj *obj, music_info *info)
{
	int retvalue = 0;
	if ((info == NULL) || (obj == NULL)) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
		goto end;
	}

	obj->cur_music = info;

	if (NULL == info->url) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
		goto end;
	}
	if (obj->cur_num >= obj->max) {
		LIST *tmp = &obj->head.list;
		music_info *m;
		while (!is_list_last(tmp)) {
			m = list_entry(tmp, music_info, list);
			tmp = tmp->next;
		}
		m = list_entry(tmp, music_info, list);
		if (m->url != NULL) {
				music_list_delete(obj, &m);
				obj->cur_num--;
		} else {
			printf("[%s %s %d] last node is head\n", __FILE__, __func__, __LINE__);
		}
	}

	_list_insert_spec(&obj->head.list, &info->list);
	obj->cur_num++;
end:
	return retvalue;
}

int music_list_insert(music_obj *obj, music_info *info)
{
	int retvalue = 0;
	if ((info == NULL) || (obj == NULL)) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
		goto end;
	}

	if (info->url == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", info->url);
	}

	if (info->artist == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", info->artist);
	}

	if (info->title == NULL) {
		printf("[%s %s %d]error: head delete\n", __FILE__, __func__, __LINE__);
		return 0;
	} else {
		//printf("[%s]\n", info->title);
	}


	obj->cur_music = info;

	LIST *tmp = &obj->head.list;
	music_info *m;
	while (!is_list_last(tmp)) {
		m = list_entry(tmp, music_info, list);
#if 0
		if (m->url != NULL) {
			if (0 == strncmp(m->url, info->url, strlen(info->url))) {
				printf("[%s %s %d]\n", __FILE__, __func__, __LINE__);
				music_list_delete(obj, m);
				_list_insert_behind(&obj->head.list, &info->list);
				goto end;
			}
		}
#endif
		tmp = tmp->next;
	}
	m = list_entry(tmp, music_info, list);
	if (m->url != NULL) {
		if (0 == strncmp(m->url, info->url, strlen(info->url))) {
			//goto end;
#if 1
			music_list_delete(obj, &m);
			obj->cur_num--;
#endif
		}
	}
	_list_insert_behind(&obj->head.list, &info->list);
	obj->cur_num++;

	if (obj->cur_num > obj->max) {
		music_info *tmp = list_entry(obj->head.list.next, music_info, list);
		music_list_delete(obj, &tmp);
		obj->cur_num--;
	}
end:
	return retvalue;
}

int music_info_alloc(music_info **info, char *title, char *artist, char *url)
{
	int retvalue = 0;
	/*XXX*/
	*info = (music_info *)malloc(sizeof(music_info));
	if (*info == NULL) {
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);
		retvalue = -1;
	}
	if (title != NULL)
		(*info)->title = strdup(title);
	else
		(*info)->title = strdup(" ");

	if (artist != NULL) {
		(*info)->artist = strdup(artist);
	} else {
		(*info)->artist = strdup(" ");
	}

	if (url != NULL)
		(*info)->url = strdup(url);
	else
		(*info)->url = strdup("null");

	return retvalue;;
}

int music_list_destroy(music_obj **obj)
{
	if (*obj == NULL) {
		printf("[%s %s %d] input obj == NULL\n", __FILE__, __func__, __LINE__);
		return 0;
	}

	LIST *tmp = (*obj)->head.list.next;
	music_info *m;
	while (!is_list_last(tmp)) {
		m = list_entry(tmp, music_info, list);
		tmp = tmp->next;
		music_list_delete((*obj), &m);
	}
	m = list_entry(tmp, music_info, list);
	if (m == NULL)
		printf("error:[%s %s %d]\n", __FILE__, __func__, __LINE__);

	if (m->url != NULL) {
		music_list_delete((*obj), &m);
	}
	(*obj)->head.list.next = &(*obj)->head.list;
	(*obj)->head.list.prev = &(*obj)->head.list;
	(*obj)->max = 0;
	(*obj)->cur_num = 0;
	(*obj)->cur_music = NULL;
	free((*obj));
	*obj = NULL;
	return 0;
}
