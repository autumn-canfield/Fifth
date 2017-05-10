#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "error.h"
#include "intern.h"
#include "hash.h"

typedef struct {
	u32 str;
	u32 h;
} string_entry;

static string_entry *table;
static u32 table_len = 0x1000; //Must be a power of two
static char *data;
static u32 data_index = 0;
static u32 size = 0x1000*16;

void init_intern_table()
{
	table = malloc(sizeof(string_entry)*table_len);
	data = malloc(size);
}

static inline u32 probe_count(u32 h, u32 i)
{
	return (table_len+i-(h&(table_len-1)))&(table_len-1);
}

u32 intern(const char *str, int len)
{
	u32 n;
	bool added_str = false;
	u32 new_hash =  hash(str, len);
	string_entry e = {new_hash, data_index};
	if(data_index+len >= size)
		data = realloc(data, (size=size*3/2));

	for(n=0; n<table_len; n++)
	{
		u32 i = (e.h+n)%table_len;
		if(table[i].h == 0)
		{
			table[i] = e;
			if(!added_str)
			{
				strcpy(&data[data_index], str);
				data_index += len+1;
			}
			return new_hash;
		}
		if(table[i].h == e.h)
		{
			if(strcmp(&data[table[i].str], &data[e.str])!=0)
				error("Strings '%s' and '%s' have the same hash!\n",
						&data[table[i].str], &data[e.str]);
			return new_hash;
		}
		if(probe_count(table[i].h, i) < n)
		{
			if(!added_str)
			{
				added_str = true;
				strcpy(&data[data_index], str);
				data_index += len+1;
			}
			string_entry tmp = table[i];
			table[i] = e;
			e = tmp;
			n = probe_count(table[i].h, i);
		}
	}
	error("Intern table overflowed! (x%x)\n", table_len);
	return 0;
}

