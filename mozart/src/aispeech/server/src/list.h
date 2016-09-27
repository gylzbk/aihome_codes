#ifndef __LIST_H__
#define __LIST_H__

#define U32    unsigned int
#define S32    signed int
#define U16    unsigned short
#define S16    signed short
#define U8     unsigned char
#define S8     signed char
#define BOOL   U8
#define ULONG  unsigned long

#define list_entry(node, type, member) \
	((type *)((U8*)(node) - (U32)(&((type *)0)->member)))

typedef struct LIST {
	struct LIST *prev;
	struct LIST *next;
} LIST;

void list_init(LIST *head);
void list_insert(LIST *head, LIST *node);
void list_insert_spec(LIST * head, LIST *node);
void list_delete(LIST *node);
BOOL is_list_last(LIST *node);
void list_insert_behind(LIST *head, LIST *node);

#endif
