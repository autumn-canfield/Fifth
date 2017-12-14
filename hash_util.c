#include <stdio.h>
#include <string.h>
#include "types.h"
#include "hash.h"

int main()
{
	char buf[256];
	while(!feof(stdin))
	{
		if(!fgets(buf, 256, stdin) && !feof(stdin))
			return 1;
		int i = 0;
		while(buf[i] != '\n' && buf[i] != 0) i++;
		if(buf[i] == '\n') buf[i] = 0;
		printf("0x%08x //%s\n", hash(buf, i), buf);
	}
	return 0;
}

