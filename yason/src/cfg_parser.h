#ifndef CFG_PARSER_H
#define CFG_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "yason_element.h"
    #include "../../../listlib/include/listlib.h"


    #define CFG_TOKEN      "=\r\n[],#"
    #define CFG_TOKEN_LENG 7


	typedef struct _CfgContentPart
	{
		char       Token;
		int        Level;
		int        Line;
		String     Content;
	}
	CfgContentPart;



	String* cfg_render(Element* root)
	{
		String* content = string_new();

		if (root->Children.Count > 0)
		{
			int ix = 0;
			while (ix < root->Children.Count)
			{
				if (ix > 0) string_append(content, "\n");

				Element* element = root->Children.Items[ix];

				string_append(content, "[");
				string_append(content, element->Name.Data);
				string_append(content, "]\n");

				int iz = 0;
				while(iz < element->Children.Count)
				{
					Element* item = element->Children.Items[iz];
					string_append(content, item->Name.Data);
					string_append(content, "=");
					string_append(content, item->Value.Data);
					string_append(content, "\n");
					iz++;
				}
				ix++;
			}
		}
		return content;
	}


	static Element* cfg_parse_elements(const char* content, const int length)
	{
		Element* root  = yason_element_new();
		root->Type     = NODE_TYPE_ROOT;
		root->TreeType = TREE_TYPE_CFG;
		root->Level    = -1;

		Element* current = 0;

		int current_position = 0;
		int ix = 0;
		int step = 0;
		int session_begin  = -1;
		int session_end    = -1;
		int value_begin    = -1;
		int start_line     = -1;

		while (ix < length)
		{
			int p = string_index_first(content, length, CFG_TOKEN, CFG_TOKEN_LENG, ix, &current_position);

			if (p >= 0)
			{
				if (p == 0)// = separador de valor
				{
					// marcar posicao
					value_begin = current_position + 1;
					ix = current_position + 1;
					continue;
				}
				else if (p == 1 || p == 2)
				{
					if (step == 2)
					{
						current = yason_element_new();
						current->Type = NODE_TYPE_ARRAY;
						string_append_sub(&current->Name, content, length, session_begin, session_end - session_begin + 1);
						yason_element_array_add(&root->Children, current);
						step = 3;
					}
					else if (step == 3)
					{
						// linhas de valores
						if (value_begin >= 0)
						{
							Element* item = yason_element_new();
							item->Type = NODE_TYPE_FIELD;
							int leng = value_begin - start_line -1;
							int xeng = current_position - value_begin;
							string_append_sub(&item->Name, content, length, start_line, leng);
							string_append_sub(&item->Value, content, length, value_begin, xeng);
							yason_element_array_add(&current->Children, item);
						}

						value_begin = -1;
					}

					start_line = current_position +1;
					ix         = current_position + 1;
					continue;
				}
				else if (p == 3)// [
				{
					ix = current_position +1;
					session_begin = ix;
					step = 1;
					continue;
				}
				else if (p == 4)// ]
				{
					if (step == 1)
					{
						session_end = current_position -1;
						ix = current_position + 1;
						step = 2;
						continue;
					}
				}

				ix = current_position + 1;
			}
			else break;
		}

		if (step == 3)
		{
			if (value_begin >= 0)
			{
				Element* item = yason_element_new();
				item->Type = NODE_TYPE_FIELD;
				int leng = value_begin - start_line - 1;
				int xeng = length - value_begin;
				string_append_sub(&item->Name, content, length, start_line, leng);
				string_append_sub(&item->Value, content, length, value_begin, xeng);
				yason_element_array_add(&current->Children, item);
			}
		}

		return root;
	} 


	Element* cfg_parse(const char* content, int length)
	{
		Element* root = cfg_parse_elements(content, length);
		return root;
	}


#ifdef __cplusplus
}
#endif

#endif /* JSON_PARSER */






