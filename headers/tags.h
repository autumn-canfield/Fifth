#pragma once

#define tag_eol  0x1
#define tag_bool 0x3
#define tag_char 0x5
#define tag_sym  0x7
#define tag_list 0x9
#define tag_arr  0xb
#define tag_ptr  0xd
#define tag_fun  0xf

#define tag_u8  0x10
#define tag_i8  0x13
#define tag_u16 0x15
#define tag_i16 0x17
#define tag_u32 0x19
#define tag_i32 0x1b
#define tag_u64 0x1d
#define tag_i64 0x1f

bool typep(u64 v);

