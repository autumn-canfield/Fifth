#include "types.h"
#include "hash.h"

#define ROTL32(x, r) ((x << r) | (x >> (32 - r)))

//Murmur3 hash
//https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
u32 hash(const char *key, int len)
{
	const u8 *data = (const u8*)key;
	const int nblocks = len/4;
	const u32 c1 = 0xcc9e2d51;
	const u32 c2 = 0x1b873593;
	u32 h1 = 0xdcd6549a; //seed
	const u32 *blocks = (const u32 *)(data + nblocks*4);
	int i;

	for(i = -nblocks; i; i++)
	{
		u32 k1 = blocks[i];
		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;
		h1 ^= k1;
		h1 = ROTL32(h1,13); 
		h1 = h1*5+0xe6546b64;
	}

	const u8 * tail = (const u8*)(data + nblocks*4);
	u32 k1 = 0;

	switch(len & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
				k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};

	h1 ^= len;
	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6b;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;

	return h1;
} 

