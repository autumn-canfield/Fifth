#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "error.h"
#include "intern.h"
#include "hash.h"

static inline u32 probe_count(u32 len, u32 h, u32 i)
{
	return (len+i-(h%len))%len;
}

u32 intern(intern_table *t, const char *str, int len)
{
	u32 n;
	bool added_str = false;
	u32 new_hash =  hash(str, len);
	string_entry e = {t->data_index, new_hash};
	if(t->data_index+len >= t->data_size)
		t->data = realloc(t->data, (t->data_size*=2));

	for(n=0; n<t->len; n++)
	{
		u32 i = (e.h+n)%t->len;
		if(t->entries[i].h == 0)
		{
			if(!added_str)
			{
				strcpy(&t->data[t->data_index], str);
				t->data_index += len+1;
			}
			t->entries[i] = e;
			return new_hash;
		}
		if(t->entries[i].h == e.h)
		{
			if(strcmp(&t->data[t->entries[i].str],
						added_str ? &t->data[e.str] : str )!=0)
				error("Strings '%s' and '%s' have the same hash!\n",
						&t->data[t->entries[i].str], &t->data[e.str]);
			return new_hash;
		}
		if(probe_count(t->len, t->entries[i].h, i) < n)
		{
			if(!added_str)
			{
				added_str = true;
				strcpy(&t->data[t->data_index], str);
				t->data_index += len+1;
			}
			string_entry tmp = t->entries[i];
			t->entries[i] = e;
			e = tmp;
			n = probe_count(t->len, t->entries[i].h, i);
		}
	}
	error("Intern table overflowed! (x%x)\n", t->len);
	return 0;
}

