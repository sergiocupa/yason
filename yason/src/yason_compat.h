#ifndef YASON_COMPAT_H
#define YASON_COMPAT_H

#include "../submodules/xplatbase/Xplatbase/Xplatbase/include/xplatbase.h"
#include "../submodules/xplatbase/Xplatbase/Xplatbase/src/numeric.h"

typedef struct YasonStringArray {
    int Max;
    int Count;
    StringX** Items;
} YasonStringArray;

StringX* yason_string_new(void);
void yason_string_append(StringX* dst, const char* data);
void yason_string_append_char(StringX* dst, char data);
void yason_string_append_sub(StringX* dst, const char* data, int data_length, int start, int count);
void yason_string_init_sub(StringX* dst, const char* data, int data_length, int start, int count);
int yason_string_with_content(const StringX* str);
int yason_string_equals(const StringX* str, const char* data);
int yason_string_equals_char(const char* a, const char* b);
int yason_string_equals_char_range(const char* a, const char* b, int start, int count);
char* yason_string_to_upper_copy(const char* data);
int yason_string_index_end_char(const char* data, char token);
int yason_string_index_first(const char* data, int data_length, const char* tokens, int token_length, int start, int* position);
int yason_string_token_count(const char* data, int data_length, char token, int start, int count, int* position);
YasonStringArray* yason_string_split_first_char(const char* data, int length, const char* tokens, int token_length);
void yason_string_trim_end_by_first_char(StringX* str, const char* tokens);
ListX* yason_list_create(uint64 type_size);
void yason_list_release(ListX* list);
int file_write_text(const char* path, char* content, int length);
int file_read_text(const char* path, char** out, int* out_length);

#endif
