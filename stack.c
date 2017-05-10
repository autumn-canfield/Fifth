#include <stdlib.h>
#include "types.h"
#include "error.h"
#include "stack.h"

static u64 *stack;
static u32 i = 0;
static u32 size = 0x200;

void init_stack()
{
	stack = malloc(sizeof(u64)*size);
}

void push(u64 value)
{
	if(i+1==size)
		stack = realloc(stack, (size=size*3/2)*sizeof(u64));
	stack[i++] = value;
}

u64 pop()
{
	if(!i)
		error("Stack underflow!\n");
	return stack[--i];
}

