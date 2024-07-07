#include "../include/listlib.h"
#include <assert.h>
#include <stdlib.h>


#define INITIAL_LIST_LENGTH 100



List* list_create(int item_size)
{
	List* ar = (List*)malloc(sizeof(List));

	ar->Length = 0;
	ar->MaxLength = INITIAL_LIST_LENGTH;
	ar->ItemSize = item_size;
	ar->Data = (void**)malloc(ar->MaxLength * sizeof(void*));
	return ar;
}

void list_add(List* _this, void* item, int item_size)
{
	if (_this)
	{
		if (item_size != _this->ItemSize)
		{
			assert(0);
		}

		if (_this->Length >= _this->MaxLength)
		{
			_this->MaxLength = ((_this->Length + item_size) + _this->MaxLength) * 2;
			_this->Data = (void**)realloc((void**)_this->Data, _this->MaxLength * sizeof(void*));
		}

		_this->Data[_this->Length] = item;
		_this->Length++;
	}
}

