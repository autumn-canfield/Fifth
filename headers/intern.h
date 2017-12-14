#pragma once
#include <stdlib.h>

typedef struct {
	u32 str;
	u32 h;
} string_entry;

typedef struct {
	string_entry *entries;
	char *data;
	u32 len;
	u32 data_index;
	u32 data_size;
} intern_table;

static inline intern_table* create_intern_table(u32 len)
{
	intern_table *t;
	t  = malloc(sizeof(intern_table));
	t->entries = malloc(sizeof(string_entry)*len);
	t->len = len;
	t->data = malloc(len*16);
	t->data_index = 0;
	t->data_size = len*16;
	return t;
}

u32 intern(intern_table *t, const char *str, int len);

