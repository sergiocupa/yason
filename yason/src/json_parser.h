//  MIT License – Modified for Mandatory Attribution
//  
//  Copyright(c) 2025 Sergio Paludo
//
//  github.com/sergiocupa
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files, 
//  to use, copy, modify, merge, publish, distribute, and sublicense the software, including for commercial purposes, provided that:
//  
//     01. The original author’s credit is retained in all copies of the source code;
//     02. The original author’s credit is included in any code generated, derived, or distributed from this software, including templates, libraries, or code - generating scripts.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.


#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

	#include "yason_element.h"
	#include "listlib.h"
	#include "stringlib.h"
    #include <string.h>
    #include <stdlib.h>

    #define JSON_TOKEN      "\":,{}[]"
    #define JSON_TOKEN_LENG 7


	typedef struct JsonTokenContent
	{
		char   Token;
		String Content;
	}
	JsonTokenContent;

	static Element* json_parse_object(List* elements, int* index);


	static JsonTokenContent* json_create_token_content(char token, const char* content, int content_length, int position, int count)
	{
		JsonTokenContent* ma = (JsonTokenContent*)malloc(sizeof(JsonTokenContent));
		memset(ma, 0, sizeof(JsonTokenContent));

		ma->Token = token;

		string_init_sub(&ma->Content, content, content_length, position, count);

		return ma;
	}


	static List* json_index_tokens(const char* content, const int length)
	{
		List* list = list_create(sizeof(JsonTokenContent));

		int current_position = 0;
		int ix = 0;

		while (ix < length)
		{
			int p = string_index_first(content, length, JSON_TOKEN, JSON_TOKEN_LENG, ix, &current_position);

			if (p >= 0)
			{
				int prev_content_count = current_position - ix;

				JsonTokenContent* element = json_create_token_content(JSON_TOKEN[p], content, length, ix, prev_content_count);

				list_add(list, element, sizeof(JsonTokenContent));

				ix = current_position + 1;
			}
			else break;
		}

		return list;
	}


	static void json_create_field(Element* obj, JsonTokenContent* name_element, JsonTokenContent* value_element, int is_string)
	{
		Element* field  = yason_element_new();
		field->TreeType = TREE_TYPE_JSON;
		field->Type     = NODE_TYPE_FIELD;
		field->IsString = is_string;

		if (name_element)
		{
			string_append_sub(&field->Name, name_element->Content.Data, name_element->Content.Length, 0, name_element->Content.Length);
			string_trim(&field->Name);
		}

		if (value_element)
		{
			string_append_sub(&field->Value, value_element->Content.Data, value_element->Content.Length, 0, value_element->Content.Length);
		}
		
		string_trim(&field->Value);
		yason_element_array_add(&obj->Children, field);
	}

	static Element* json_parse_array(List* elements, int* index)
	{
		Element* arra  = yason_element_new();
		arra->TreeType = TREE_TYPE_JSON;
		arra->Type     = NODE_TYPE_ARRAY;

		int last_value = 0;

		int ix = (*index);
		while (ix < elements->Length)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Data[ix];

			if (element->Token == '{')
			{
				ix++;
				Element* no = json_parse_object(elements, &ix);
				yason_element_array_add(&arra->Children, no);
				last_value = 0;
				continue;
			}
			else if (element->Token == '"')// array de valor string
			{
				ix++;
				JsonTokenContent* element2 = (JsonTokenContent*)elements->Data[ix];

				if (element2->Token == '"')
				{
					json_create_field(arra, 0, element2, 1);
					last_value = 1;
				}
				else
				{
					// Erro
				}
			}
			else if(element->Token == ',')// novo item
			{
				json_create_field(arra, 0, element, 0);
				last_value = 0;
			}
			else if (element->Token == ']')
			{
				if (!last_value && string_with_content(&element->Content))
				{
					json_create_field(arra, 0, element, 0);
				}
				ix++;
				break;
			}

			ix++;
		}

		(*index) = ix;

		return arra;
	}

	static Element* json_parse_object(List* elements, int* index)
	{
		Element* obj  = yason_element_new();
		obj->TreeType = TREE_TYPE_JSON;
		obj->Type     = NODE_TYPE_OBJECT;

		int ix = (*index);
		while (ix < elements->Length)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Data[ix];

			if (element->Token == '"')
			{
				ix++;
				JsonTokenContent* element2 = (JsonTokenContent*)elements->Data[ix];

				if (element2->Token == '"')
				{
					ix++;
					JsonTokenContent* element3 = (JsonTokenContent*)elements->Data[ix];

					if (element3->Token == ':')
					{
						ix++;
						JsonTokenContent* element4 = (JsonTokenContent*)elements->Data[ix];

						if (element4->Token == '"')// valor string
						{
							ix++;
							JsonTokenContent* element5 = (JsonTokenContent*)elements->Data[ix];

							if (element5->Token == '"')
							{
								ix++;
								JsonTokenContent* element6 = (JsonTokenContent*)elements->Data[ix];

								json_create_field(obj, element2, element5, 1);

								if (element6->Token == ',')
								{
									ix++;
									continue;
								}
								else if (element6->Token == '}')
								{
									ix++;
									break;
								}
								else
								{
									// erro
								}
							}
							else 
							{
								// erro
							}
						}
						else if (element4->Token == ',')// valor nao string
						{
							json_create_field(obj, element2, element4, 0);
						}
						else if (element4->Token == '[')// inicio de array
						{
							ix++;
							Element* ar = json_parse_array(elements, &ix);

							string_append_sub(&ar->Name, element2->Content.Data, element2->Content.Length, 0, element2->Content.Length);
							string_trim(&ar->Name);

							yason_element_array_add(&obj->Children, ar);
							continue;
						}
						else if (element4->Token == '{')// campo objeto
						{
							ix++;
							Element* no = json_parse_object(elements, &ix);

							string_append_sub(&no->Name, element2->Content.Data, element2->Content.Length, 0, element2->Content.Length);
							string_trim(&no->Name);

							yason_element_array_add(&obj->Children, no);
							continue;
						}
						else if (element4->Token == '}')// fim de objeto
						{
							// TO-TO: implementar campo no fim do objeto
							ix++;
							break;
						}
					}
					else
					{
						// erro
					}
				}
				else
				{
					// erro
				}
			}
			else if (element->Token == '}')
			{
				ix++;
				break;
			}
			ix++;
		}

		(*index) = ix;

		return obj;
	}


	static Element* json_parse_elements(List* elements)
	{
		Element* root = 0;

		int ix = 0;
		while (ix < elements->Length)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Data[ix];

			if (element->Token == '{')
			{
				ix++;
				root = json_parse_object(elements, &ix);
				break;
			}
			else if (element->Token == '[')
			{
				ix++;
				root = json_parse_array(elements, &ix);
				break;
			}

			ix++;
		}
		return root;
	}


	Element* json_parse(const char* content, int length)
	{
		List*    elements = json_index_tokens(content, length);
		Element* root     = json_parse_elements(elements);
		return root;
	}

#ifdef __cplusplus
}
#endif

#endif /* JSON_PARSER */