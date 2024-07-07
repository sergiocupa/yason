#ifndef STRING_LIB_H
#define STRING_LIB_H

#ifdef __cplusplus
extern "C" {
#endif



#include "../../platformlib/include/platform.h"



typedef struct _String
{
	int   MaxLength;
	int   Length;
	char* Data;
}
String;




PLATFORM_API void string_init(String* ni);
PLATFORM_API void string_init_length(String* ni, int max_length);
PLATFORM_API void string_resize(String* _this, int max_length);
PLATFORM_API String* string_new();
PLATFORM_API String* string_copy(String* _this);
PLATFORM_API String* string_new_length(int max_length);
PLATFORM_API void string_release_data(String* ar);
PLATFORM_API void string_release(String* ar);
PLATFORM_API char* string_to_upper_copy_achar(const char* content);
PLATFORM_API String* string_to_upper_copy(String* _this);
PLATFORM_API int string_with_content(String* _this);
PLATFORM_API void string_trim(String* _this);
PLATFORM_API void string_init_sub(String* _this, const char* data, int data_length, int data_start, int data_count);
PLATFORM_API void string_append_sub(String* _this, const char* data, int data_length, int data_start, int data_count);
PLATFORM_API int string_equals_char_range(const char* s1, const char* s2, const int s2_start, const int s2_count);
PLATFORM_API int string_equals_char(const char* s1, const char* s2);
PLATFORM_API int string_equals(String* s1, const char* s2);
PLATFORM_API void string_append_char(String* _this, const char data);
PLATFORM_API void string_append(String* _this, const char* data);
PLATFORM_API String* string_append_format(const char* format, ...);
PLATFORM_API inline void string_append_s(String* _this, String* data);
PLATFORM_API inline void string_sub(const char* content, const int content_length, const int start, const int count, const int initialize, String* target);
PLATFORM_API inline String* string_sub_new(const char* content, const int content_length, const int start, const int count);
PLATFORM_API inline int string_token_count(const char* data, const int data_length, const char token, const int start, const int count, int* position);
PLATFORM_API inline int string_index_of_char(const char* data, const int data_length, const char token, const int start, const int count);
PLATFORM_API inline int string_index_end_char(const char* data, const char token);
PLATFORM_API inline int string_index_of(const char* data, const int data_length, const char* token, const int token_length, const int start);
PLATFORM_API inline int string_index_first(const char* data, const int data_length, const char* token, const int token_length, const int start, int* position);




#ifdef __cplusplus
}
#endif

#endif /* STRING_LIB */