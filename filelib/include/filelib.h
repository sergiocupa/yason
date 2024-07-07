#ifndef FILE_LIB_H
#define FILE_LIB_H

#ifdef __cplusplus
extern "C" {
#endif
   
#include "../../platformlib/include/platform.h"



PLATFORM_API inline int file_write_text(const char* path_file, char* content, int length);
PLATFORM_API inline int file_read_text(const char* path_file, char** out, int* out_length);



#ifdef __cplusplus
}
#endif

#endif /* FILE_LIB */