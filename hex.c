#include "types.h"
#include "hex.h"

static inline u8 parse_nibble(char n)
{
	if(n&0x10)
		return n&0x0f;
	else
		return n-0x57;
}

u64 parse_hex(char *str)
{
	u64 i, tmp = 0;
	for(i=0; str[i]&&i<16; ++i)
		tmp = (tmp<<4) | parse_nibble(str[i]);
	return tmp;
}

bool is_valid_hex_string(char *str)
{
	u32 i;
	for(i=0; str[i]; ++i)
	{
		char c = str[i];
		if(c<48||(c>57&&c<97)||c>102)
			return false;
	}
	return true;
}

