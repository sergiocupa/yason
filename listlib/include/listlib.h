#ifndef LIST_LIB_H
#define LIST_LIB_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "../../platformlib/include/platform.h"


typedef struct _List
{
	int    MaxLength;
	int    ItemSize;
	int    Length;
	void** Data;
}
List;



PLATFORM_API List* list_create(int item_size);
PLATFORM_API void list_add(List* _this, void* item, int item_size);


#ifdef __cplusplus
}
#endif

#endif /* LIST_LIB */