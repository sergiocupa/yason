#ifndef YASON_ELEMENT_H
#define YASON_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../stringlib/include/stringlib.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define YASON_INITIAL_NODE_COUNT 100

	typedef enum _ElementType
	{
		NODE_TYPE_UNKNOWN  = 0,
		NODE_TYPE_ROOT     = 1,
		NODE_TYPE_NULL     = 2,
		NODE_TYPE_SCALAR   = 3,
		NODE_TYPE_SEQUENCE = 4,
		NODE_TYPE_MAP      = 5,
		NODE_TYPE_OBJECT   = 6,
		NODE_TYPE_FIELD    = 7,
		NODE_TYPE_ARRAY    = 8
	}
	ElementType;

	typedef enum _TreeTypeOption
	{
		TREE_TYPE_UNKNOWN = 0,
		TREE_TYPE_JSON    = 1,
		TREE_TYPE_YAML    = 2,
		TREE_TYPE_XML     = 3,
		TREE_TYPE_HTML    = 4
	}
	TreeTypeOption;

	typedef struct _ElementArray ElementArray;
	typedef struct _Element      Element;

	struct _ElementArray
	{
		int       MaxCount;
		int       Count;
		Element** Items;
	};

	struct _Element
	{
		int            Index;
		int            Level;
		int            Pad;
		int            IsString;
		int            IsMap;
		int            InLine;
		int            Line;
		Element*       Parent;
		TreeTypeOption TreeType;
		ElementType    Type;
		String         Name;
		String         Value;
		String         Comment;
		ElementArray   Children;
	};




	// Criar lista de Nós
	static void yason_element_array_init(ElementArray* ar)
	{
		ar->Items = (Element**)malloc(sizeof(Element*) * YASON_INITIAL_NODE_COUNT);
		ar->Count = 0;
		ar->MaxCount = YASON_INITIAL_NODE_COUNT;
	}
	static ElementArray* yason_node_array_new()
	{
		ElementArray* ar = (ElementArray*)malloc(sizeof(ElementArray));
		ar->Items = (Element**)malloc(sizeof(Element*) * YASON_INITIAL_NODE_COUNT);
		ar->Count = 0;
		ar->MaxCount = YASON_INITIAL_NODE_COUNT;
		return ar;
	}
	static void yason_element_array_release(ElementArray** ar, int only_data)
	{
		ElementArray* a = *ar;
		if (!a)
		{
			int ix = 0;
			while (ix < a->Count)
			{
				free(a->Items[ix]);
				ix++;
			}
			free(a->Items);

			if (!only_data)
			{
				free((*ar));
				*ar = 0;
			}
		}
	}
	static void yason_element_array_add(ElementArray* _this, Element* node)
	{
		if (_this)
		{
			if (_this->Count >= _this->MaxCount)
			{
				_this->MaxCount *= 2;
				size_t nsz = sizeof(Element*) * _this->MaxCount;
				Element** items = (Element**)realloc(_this->Items, nsz);

				if (!items) assert(0);

				_this->Items = items;
			}

			_this->Items[_this->Count] = node;
			_this->Count++;
		}
	}
	static void yason_element_array_transfer(ElementArray* source, ElementArray* destination, int deallocate_source)
	{
		if (source->Count > destination->MaxCount)
		{
			destination->MaxCount = source->Count;
			size_t nsz = sizeof(Element*) * destination->MaxCount;
			Element** items = (Element**)realloc(destination->Items, nsz);

			if (!items) assert(0);

			destination->Items = items;
		}

		destination->Count = 0;
		while (destination->Count < source->Count)
		{
			destination->Items[destination->Count] = source->Items[destination->Count];
			destination->Count++;
		}

		if (deallocate_source)
		{
			free(source);
		}
	}


	static void yason_element_init(Element* ar)
	{
		memset(ar, 0, sizeof(Element));
		string_init(&ar->Name);
		string_init(&ar->Value);
		yason_element_array_init(&ar->Children);
	}
	static Element* yason_element_new()
	{
		Element* ar = (Element*)malloc(sizeof(Element));

		yason_element_init(ar);

		return ar;
	}




#ifdef __cplusplus
}
#endif

#endif /* YASON_ELEMENT */