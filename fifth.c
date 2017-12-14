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

table syms = initialize_table(0x10000);
intern_table *i_table;

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
u32 string_data_index = 0;

u32 read_string_until_char(char *dst, const char *src, char c)
{
	u32 i = 0;
	while(src[i] && src[i] != c)
	{
		dst[i] = src[i];
		i++;
	}
	dst[i++] = (char)0;
	return i;
}

bool predicate = false;

void eval_function(const char *name, u32 len);

#define LOCAL_MAX 256
void eval_string(const char *str)
{
	bool in_comment = false;
	u32 string_index = 0;
	char local[LOCAL_MAX];
	u32 local_index = 0;

	while(str[string_index])
	{
		char c = str[string_index++];
		u32 token_length, len;

		if(!in_comment && c=='\\')
			in_comment = true;
		if(in_comment && c=='\n')
			in_comment = false;
		if(in_comment) continue;

		switch(c)
		{
			case '{':
				string_index += read_whitespace(&str[string_index]);
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				table_set_value(syms, intern(i_table, token_buffer, token_length),
						(u64)&string_data[string_data_index]);
				len = read_string_until_char(&string_data[string_data_index],
					&str[string_index], '}');
				if((string_data_index += len) == countof(string_data)-1)
					error("Error: string memory exhausted!");
				string_index += len;
				++string_index; //Consume ending brace.
				break;
			case '"':
				push((u64)&string_data[string_data_index]);
				len = read_string_until_char(&string_data[string_data_index],
					&str[string_index], '"');
				if((string_data_index += len) == countof(string_data)-1)
					error("Error: string memory exhausted!");
				string_index += len;
				++string_index; //Consume ending quote.
				break;
			case '(':
				push((u64)&local[local_index]);
				len = read_string_until_char(&local[local_index],
					&str[string_index], ')');
				if((local_index += len) == LOCAL_MAX-1)
					error("Error: local memory exhausted!");
				string_index += len;
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
				table_set_value(syms, intern(i_table, token_buffer, token_length), pop());
				break;
			case '.':
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				push(table_get_value(syms, intern(i_table, token_buffer, token_length)));
				break;
			case '\'':
				token_length = read_token(&str[string_index]);
				string_index += token_length;
				if(predicate)
				{
					if(token_buffer[0] == 'x' && is_valid_hex_string(&token_buffer[1]))
						push(parse_hex(&token_buffer[1]));
					else
						eval_function(token_buffer, token_length);
				}
				break;
			case ' ':
			case '\n':
			case '\t':
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
	u32 h = intern(i_table, name, len);
	long t, l;
	u64 x, y, z;
	switch(h)
	{
		//Hashes were generated using hash_util.
		case 0xcb234ace: //printx
			printf("x%lx", (u64)pop());
			fflush(stdout);
			break;
		case 0xbba9595e: //dup
			x = pop();
			push(x);
			push(x);
			break;
		case 0xd1d59ea3: //drop
			(void)pop();
			break;
		case 0x12637bc5: //swap
			x = pop();
			y = pop();
			push(x);
			push(y);
			break;
		case 0xc7973296: //over
			y = pop();
			x = pop();
			push(x);
			push(y);
			push(x);
			break;
		case 0x9f120784: //if0
			predicate = (peek() == 0);
			break;
		case 0xe22b58fb: //else
			predicate = predicate ? false : true;
			break;
		case 0x468f071f: //loop
			x = pop();
			while(eval_string((const char *)x), predicate){}
			break;
		case 0xeb6e48c4: //for (fun count --)
			x = pop();
			y = pop();
			for(z=0; z<x; z++)
			{
				push(z);
				eval_string((const char *)y);
			}
			break;
		case 0x9acf688f: //prints
			printf("%s", (char*)pop());
			break;
		case 0xb99662fd: //strcmp
			x = pop();
			y = pop();
			if(strcmp((char *)x, (char *)y)==0)
				push(1);
			else
				push(0);
			break;
		case 0x600a2926: //hash (str -- hash)
			x = pop();
			push(hash((char*)x, strlen((char*)x)));
			break;
		case 0xa5600c7c: //create-intern-table (len -- i_table)
			x = pop();
			push((u64)create_intern_table(x));
			break;
		case 0xe8f21fac: //intern (str i_table -- hash)
			x = pop();
			y = pop();
			push(intern((intern_table*)x, (char*)y, strlen((char*)y)));
			break;
		case 0xc06fbc63: //create-table (size -- table)
			x = pop();
			y = (u64)malloc(sizeof(table));
			*((table*)y) = create_table(x);
			push(y);
			break;
		case 0x0dd2b3a2: //table-get (key table -- value)
			x = pop();
			y = pop();
			push(table_get_value(*((table*)x), y));
			break;
		case 0xc9b53e70: //table-set (key value table --)
			x = pop();
			y = pop();
			table_set_value(*((table*)x), pop(), y);
			break;
		case 0x0479f225: //fopenr (path -- file-handle)
			x = pop();
			push((u64)fopen((char *)x, "rb+"));
			break;
		case 0xb448a90c: //fopenw (path -- file-handle)
			x = pop();
			push((u64)fopen((char *)x, "wb+"));
			break;
		case 0xacde0aec: //fopena (path -- file-handle)
			x = pop();
			push((u64)fopen((char *)x, "ab+"));
			break;
		case 0xadde8890: //fread (ptr size file-handle -- bytes-read)
			x = pop();
			y = pop();
			push(fread((void *)x, y, 1, (FILE *)pop()));
			break;
		case 0x7552ad22: //fwrite (ptr size file-handle -- )
			x = pop();
			y = pop();
			fwrite((void *)x, y, 1, (FILE *)pop());
			break;
		case 0xc3ac72b8: //fclose
			fclose((FILE *)pop());
			break;
		case 0x4bff2ebc: //fsize (file-handle -- file-size)
			x = pop();
			t = ftell((FILE *)x);
			fseek((FILE *)x, 0, SEEK_END);
			l = ftell((FILE *)x);
			fseek((FILE *)x, t, SEEK_SET);
			push(l);
			break;
		case 0x1defeb8c: //fseek (pos file-handle -- )
			x = pop();
			y = pop();
			fseek((FILE *)x, y, SEEK_SET); 
			break;
		case 0xe12a70ee: //ftell (file-handle -- position)
			push(ftell((FILE *)pop()));
			break;
		case 0x7dbd3108: //exit
			exit(pop());
		case 0x92c34fc1: //eval
			eval_string((char*)pop());
			break;
		case 0xe548af45: //allot
			push((u64)calloc(pop(), 1));
			break;
		case 0x8d782939: //free
			free((void*)pop());
			break;
		case 0xfbcf85c1: //! (value addr --)
			x = pop();
			*((u64*)x)=(u64)pop();
			break;
		case 0x4333ce96: //@ (addr -- value)
			push(*((u64*)pop()));
			break;
		case 0xbc8c8552: //!d
			x = pop();
			*((u32*)x)=(u32)pop();
			break;
		case 0x7249426a: //@d
			push(*((u32*)pop()));
			break;
		case 0xc7116baa: //!w
			x = pop();
			*((u16*)x)=(u16)pop();
			break;
		case 0xe5ea206f: //@w
			push(*((u16*)pop()));
			break;
		case 0x94b20459: //!b
			x = pop();
			*((u8*)x)=(u8)pop();
			break;
		case 0x42d0e7f7: //@b
			push(*((u8*)pop()));
			break;
		case 0x5aa33ea8: //+
			x = pop();
			y = pop();
			push(x+y);
			break;
		case 0x16e35704: //-
			y = pop();
			x = pop();
			push(x-y);
			break;
		case 0x183630f4: //*
			x = pop();
			y = pop();
			push(x*y);
			break;
		case 0x293310a9: ///
			y = pop();
			x = pop();
			push(x/y);
			break;
		case 0x447d381a: //%
			y = pop();
			x = pop();
			push(x%y);
			break;
		case 0xdb39bba3: //>>
			y = pop();
			x = pop();
			push(x>>y);
			break;
		case 0xe7be138c: //<<
			y = pop();
			x = pop();
			push(x<<y);
			break;
		case 0x4077392c: //&
			x = pop();
			y = pop();
			push(x&y);
			break;
		case 0x5191dd95: //|
			x = pop();
			y = pop();
			push(x|y);
			break;
		case 0x7eb1049d: //^
			x = pop();
			y = pop();
			push(x^y);
			break;
		case 0xf1c9b157: //~
			push(~pop());
			break;
		default:
			// Must be a user defined function.
			x = table_get_value(syms, intern(i_table, name, len));
			if(x)
				eval_string((char *)x);
			else
				error("Error: tried to call undefined function '%s'!\n", name);
	}
}

int main(int argc, char **argv)
{
	i_table = create_intern_table(0x1000);
	intern(i_table, "printx", 6);
	intern(i_table, "dup", 3);
	intern(i_table, "drop", 4);
	intern(i_table, "swap", 4);
	intern(i_table, "over", 4);
	intern(i_table, "if0", 3);
	intern(i_table, "prints", 6);
	intern(i_table, "strcmp", 6);
	intern(i_table, "hash", 4);
	intern(i_table, "create_intern_table", 19);
	intern(i_table, "intern", 6);
	intern(i_table, "create_table", 12);
	intern(i_table, "table_get", 9);
	intern(i_table, "table_set", 9);
	intern(i_table, "fopenr", 6);
	intern(i_table, "fopenw", 6);
	intern(i_table, "fopena", 6);
	intern(i_table, "fread", 5);
	intern(i_table, "fwrite", 6);
	intern(i_table, "fclose", 6);
	intern(i_table, "fsize", 5);
	intern(i_table, "fseek", 5);
	intern(i_table, "ftell", 5);
	intern(i_table, "exit", 4);
	intern(i_table, "eval", 4);
	intern(i_table, "allot", 5);
	intern(i_table, "!", 1);
	intern(i_table, "@", 1);
	intern(i_table, "!d", 2);
	intern(i_table, "@d", 2);
	intern(i_table, "!b", 2);
	intern(i_table, "@b", 2);
	intern(i_table, "+", 1);
	intern(i_table, "-", 1);
	intern(i_table, "*", 1);
	intern(i_table, "/", 1);
	intern(i_table, "%", 1);
	intern(i_table, ">>", 2);
	intern(i_table, "<<", 2);
	intern(i_table, "&", 1);
	intern(i_table, "|", 1);
	intern(i_table, "^", 1);
	intern(i_table, "~", 1);
	init_stack();
	if(argc < 2)
		error("Please input a file!\n");
	FILE *f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *str = malloc(len+1);
	fread(str, 1, len, f);
	str[len] = '\0';
	eval_string(str);
	return 0;
}

