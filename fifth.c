#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "error.h"
#include "stack.h"
#include "intern.h"
#include "table.h"
#include "hash.h"
#include "hex.h"

u32 read_whitespace(const char *str)
{
	u32 i = 0;
	while(str[i]==' ' || str[i]=='\n')
		++i;
	return i;
}

char token_buffer[64];
u8 buffer_index = 0;
u32 read_token(const char *str)
{
	buffer_index = 0;
	while(str[buffer_index]!=' ' && str[buffer_index]!='\n' && str[buffer_index])
	{
		token_buffer[buffer_index] = str[buffer_index];
		++buffer_index;
		if(buffer_index==countof(token_buffer)-1)
			error("Error: word '%s' exceeded maximum length!\n", token_buffer);
	}
	token_buffer[buffer_index]=0;
	return buffer_index;
}

char string_data[1048576];
u32 string_data_index;

u32 read_string_until_char(const char *str, char c)
{
	u32 str_index = 0;
	while(str[str_index] != c && str[str_index])
	{
		string_data[string_data_index] = str[str_index];
		++string_data_index;
		++str_index;
		if(string_data_index==countof(string_data)-1)
			error("Error: string memory exhausted!");
	}
	string_data[string_data_index] = (char)0;
	++string_data_index;
	return str_index;
}

table syms = initialize_table(0x10000);
void eval_function(const char *name, u32 len);

void eval_string(const char *str)
{
	bool in_comment = false;
	u32 string_index = 0;

	while(str[string_index])
	{
		char c = str[string_index++];
		u32 token_length;

		if(c=='\\')
		{
			if(in_comment) in_comment=false;
			else in_comment=true;
			continue;
		}
		if(in_comment) continue;

		switch(c)
		{
			case '{':
				string_index += read_whitespace(&str[string_index]);
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				table_set_value(syms, intern(token_buffer, token_length),
						(u64)&string_data[string_data_index]);
				string_index += read_string_until_char(&str[string_index], '}');
				++string_index; //Consume ending brace.
				break;
			case '"':
				push((u64)&string_data[string_data_index]);
				string_index += read_string_until_char(&str[string_index], '"');
				++string_index; //Consume ending quote.
				break;
			case 'x':
				token_length = read_token(&str[string_index]);
				if(!is_valid_hex_string(token_buffer)) goto switch_default;
				string_index += token_length;
				push(parse_hex(token_buffer));
				break;
			case ':':
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				table_set_value(syms, intern(token_buffer, token_length), pop());
				break;
			case '.':
				string_index += read_token(&str[string_index]);
				push(table_get_value(syms, intern(token_buffer, token_length)));
				break;
			case '\'':
				token_length = read_token(&str[string_index]);
				if(pop())
				{
					string_index += token_length;
					eval_function(token_buffer, token_length);
				}
				break;
			case ' ':
			case '\n':
				//Ignore trailing whitespace.
				break;
switch_default:
			default:
				--string_index; //First character is not a prefix.
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				eval_function(token_buffer, token_length);
				break;
		}
	}
}

void eval_function(const char *name, u32 len)
{
	u32 h = intern(name, len);
	long t, l;
	u64 x, y;
	switch(h)
	{
		//Hashes were generated using hash_util.
		case 0xcb234ace: //printx
			printf("x%lx\n", (u64)pop());
			return;
		case 0xbba9595e: //dup
			x = pop();
			push(x);
			push(x);
			return;
		case 0xd1d59ea3: //drop
			(void)pop();
			return;
		case 0x12637bc5: //swap
			x = pop();
			y = pop();
			push(x);
			push(y);
			return;
		case 0xc7973296: //over
			y = pop();
			x = pop();
			push(x);
			push(y);
			push(x);
			return;
		case 0x9f120784: //if0
			if(pop() == 0)
				push(1);
			else
				push(0);
			return;
		case 0x9acf688f: //prints
			printf("%s", (char*)pop());
			return;
		case 0x600a2926: //hash
			x = pop();
			push(hash((char*)x, strlen((char*)x)));
			return;
		case 0xb99662fd: //strcmp
			x = pop();
			y = pop();
			if(strcmp((char *)x, (char *)y)==0)
				push(1);
			else
				push(0);
			return;
		case 0x7b51ddc3: //fopen (path -- file-handle)
			x = pop();
			push((u64)fopen((char *)x, "a+"));
			return;
		case 0xadde8890: //fread (ptr size file-handle -- bytes-read)
			x = pop();
			y = pop();
			push(fread((void *)x, y, 1, (FILE *)pop()));
			return;
		case 0x7552ad22: //fwrite (ptr size file-handle -- )
			x = pop();
			y = pop();
			fwrite((void *)x, y, 1, (FILE *)pop());
			return;
		case 0xc3ac72b8: //fclose
			fclose((FILE *)pop());
			return;
		case 0x4bff2ebc: //fsize (file-handle -- file-size)
			x = pop();
			t = ftell((FILE *)x);
			fseek((FILE *)x, 0, SEEK_END);
			l = ftell((FILE *)x);
			fseek((FILE *)x, t, SEEK_SET);
			push(l);
			return;
		case 0x1defeb8c: //fseek (pos file-handle -- )
			x = pop();
			y = pop();
			fseek((FILE *)x, y, SEEK_SET); 
			return;
		case 0xe12a70ee: //ftell (file-handle -- position)
			push(ftell((FILE *)pop()));
			return;
		case 0x7dbd3108: //exit
			exit(pop());
		case 0x92c34fc1: //eval
			eval_string((char*)pop());
			return;
		case 0xe548af45: //allot
			push((u64)malloc(pop()));
			return;
		case 0xfbcf85c1: //!
			x = pop();
			*((u64*)pop())=(u64)x;
			return;
		case 0x4333ce96: //@
			push(*((u64*)pop()));
			return;
		case 0x297e8b35: //!d
			x = pop();
			*((u32*)pop())=(u32)x;
			return;
		case 0x7249426a: //@d
			push(*((u32*)pop()));
			return;
		case 0x94b20459: //!b
			x = pop();
			*((u8*)pop())=(u8)x;
			return;
		case 0x42d0e7f7: //@b
			push(*((u64*)pop())&0xff);
			return;
		case 0x5aa33ea8: //+
			x = pop();
			y = pop();
			push(x+y);
			return;
		case 0x16e35704: //-
			y = pop();
			x = pop();
			push(x-y);
			return;
		case 0x183630f4: //*
			x = pop();
			y = pop();
			push(x*y);
			return;
		case 0x293310a9: ///
			y = pop();
			x = pop();
			push(x/y);
			return;
		case 0x447d381a: //%
			y = pop();
			x = pop();
			push(x%y);
			return;
		case 0xdb39bba3: //>>
			y = pop();
			x = pop();
			push(x>>y);
			return;
		case 0xe7be138c: //<<
			y = pop();
			x = pop();
			push(x<<y);
			return;
		case 0x4077392c: //&
			x = pop();
			y = pop();
			push(x&y);
			return;
		case 0x5191dd95: //|
			x = pop();
			y = pop();
			push(x|y);
			return;
		case 0x7eb1049d: //^
			x = pop();
			y = pop();
			push(x^y);
			return;
		case 0xf1c9b157: //~
			push(~pop());
			return;
		default:
			// Must be a user defined function.
			x = table_get_value(syms, intern(name, len));
			if(x)
				eval_string((char *)x);
			else
				error("Error: tried to call undefined function '%s'!\n", name);
	}
}

int main(int argc, char **argv)
{
	init_intern_table();
	init_stack();
	if(argc < 2)
		error("Please input a file!\n");
	FILE *f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *str = malloc(len);
	fread(str, 1, len, f);
	eval_string(str);
	return 0;
}

