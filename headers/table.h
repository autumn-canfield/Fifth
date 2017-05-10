#pragma once
#include <stdlib.h>

typedef struct {
	u64 value;
	u32 key;
} table_entry;

typedef struct {
	table_entry *data;
	u32 length;
} table;

#define initialize_table(size) {.data=(table_entry[(size)]){{0, 0}}, .length=(size)}

static inline table create_table(u32 length)
{
	table t;
	t.data = malloc(sizeof(table_entry)*length);
	t.length = length;
	return t;
}
u64 table_get_value(table t, u32 key);
void table_set_value(table t, u32 key, u64 value);

