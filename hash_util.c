#include <stdio.h>
#include <string.h>
#include "types.h"
#include "hash.h"

int main(int argc, char **argv)
{
	if(argc == 1)
		puts("Prints out the hash of each argument in hexadecimal");
	int i;
	for(i=1; i<argc; i++)
	{
		printf("0x%08x\n", hash(argv[i], strlen(argv[i])));
	}
	return 0;
}

