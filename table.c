#include <stdio.h>
#include "types.h"
#include "table.h"
#include "error.h"

static inline u32 probe_count(table t, u32 key, u32 i)
{
	if((key%t.length) > i)
		return t.length+i-(key%t.length);
	else
		return i-(key%t.length);
}

u64 table_get_value(table t, u32 key)
{
	u32 n;
	for(n=0; n<t.length; n++)
	{
		u32 i = (key+n)%t.length;
		if(t.data[i].key == 0)
			return 0;
		if(probe_count(t, t.data[i].key, i) < n)
			return 0;
		if(t.data[i].key == key)
			return t.data[i].value;
	}
	return 0;
}

void table_set_value(table t, u32 key, u64 value)
{
	u32 n;
	table_entry entry = {value, key};

	for(n=0; n<t.length; n++)
	{
		u32 i = (entry.key+n)%t.length;
		if(t.data[i].key == 0)
		{
			t.data[i].key = key;
			t.data[i].value = value;
			return;
		}
		if(t.data[i].key == key)
		{
			t.data[i].value = value;
			return;
		}
		if(probe_count(t, t.data[i].key, i) < n)
		{
			table_entry tmp = t.data[i];
			t.data[i] = entry;
			entry = tmp;
			n = probe_count(t, t.data[i].key, i);
		}
	}
	error("Error: Too many elements in table! (data=%u)\n", t.data);
}

