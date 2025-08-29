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


#ifndef CFG_PARSER_H
#define CFG_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "yason_element.h"
    #include "listlib.h"


    #define CFG_TOKEN      "=[],#"
    #define CFG_TOKEN_LENG 5


	typedef struct _CfgContentPart
	{
		char       Token;
		int        Level;
		int        Line;
		String     Content;
	}
	CfgContentPart;

	typedef struct _ArrayPosition
	{
		int Start;
		int Length;
	}
	ArrayPosition;


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

					if (item->Type == NODE_TYPE_ARRAY)
					{
						if (item->Children.Count > 0)
						{
							int ic = 0;
							while (ic < (item->Children.Count-1))
							{
								Element* ia = item->Children.Items[ic];
								string_append(content, ia->Value.Data);
								string_append(content, ",");
								ic++;
							}
							Element* ib = item->Children.Items[ic];
							string_append(content, ib->Value.Data);
						}
					}
					else
					{
						string_append(content, item->Value.Data);
					}

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

		Element* current          = 0;
		int      current_position = 0;
		int      iw;

		// marcadores de elementos
		int pos_session_begin    = -1;
		int pos_session_end      = -1;
		int pos_value_begin_      = -1;
		int pos_value_end        = -1;
		int pos_comment_begin    = -1;
		ArrayPosition pos_array_item[4096];
		int pos_array_length = 0;
		memset(&pos_array_item, -1, sizeof(ArrayPosition)*4096);


		StringArray* lines = string_split_first_char(content, length, "\r\n", 2);

		if (lines->Count > 0)
		{
			int iz = 0;
			while (iz < lines->Count)
			{
				String* line = lines->Items[iz];
				string_trim_end_by_first_char(line, "\r\n");

				pos_session_begin = -1;
				pos_session_end   = -1;
				pos_value_begin_  = -1;
				pos_value_end     = -1;
				
				pos_comment_begin = -1;
				pos_array_length  = 0;

				int ix = 0;
				while (ix < line->Length)
				{
					int p = string_index_first(line->Data, line->Length, CFG_TOKEN, CFG_TOKEN_LENG, ix, &current_position);

					if (p >= 0)
					{
						if (p == 0)//      = 
						{
							pos_value_begin_ = ix;
							pos_value_end    = current_position;
							ix = current_position + 1;
							continue;
						}
						else if (p == 1)// [
						{
							pos_session_begin = current_position;
							ix = current_position + 1;
							continue;
						}
						else if (p == 2)// ]
						{
							pos_session_end = current_position;
							ix = current_position + 1;
							continue;
						}
						else if (p == 3)// ,
						{
							pos_array_item[pos_array_length].Start  = ix;
							pos_array_item[pos_array_length].Length = current_position - ix;
							pos_array_length++;
							ix = current_position + 1;
							continue;
						}
						else if (p == 4)// #
						{
							pos_comment_begin = current_position;
							ix = current_position + 1;
							break;
						}
					}
					else break;
				}

				if (pos_array_length > 0)// resto de conteudo array
				{
					ArrayPosition prev = pos_array_item[pos_array_length - 1];
					int st = prev.Start + prev.Length + 1;
					pos_array_item[pos_array_length].Start = st;
					pos_array_item[pos_array_length].Length = pos_comment_begin >= 0 ? pos_comment_begin - st : (line->Length - st);
					pos_array_length++;
				}

				if (pos_session_begin >= 0 && pos_session_end > pos_session_begin)// inicio valor
				{
					current = yason_element_new();
					current->Type = NODE_TYPE_ARRAY;
					string_append_sub(&current->Name, line->Data, line->Length, pos_session_begin + 1, pos_session_end - pos_session_begin - 1);
					yason_element_array_add(&root->Children, current);
				}
				else if (pos_value_begin_ >= 0 && pos_value_end > pos_value_begin_)
				{
					if (current != 0)
					{
						Element* item = yason_element_new();

						string_append_sub(&item->Name, line->Data, line->Length, pos_value_begin_, pos_value_end - pos_value_begin_);

						if (pos_array_length > 0)
						{
							item->Type = NODE_TYPE_ARRAY;
							iw = 0;
							while (iw < pos_array_length)
							{
								ArrayPosition ap = pos_array_item[iw];
								Element* ar = yason_element_new();
								ar->Type = NODE_TYPE_FIELD;
								string_append_sub(&ar->Value, line->Data, line->Length, ap.Start, ap.Length);
								yason_element_array_add(&item->Children, ar);
								iw++;
							}
						}
						else
						{
							item->Type = NODE_TYPE_FIELD;
							int b = pos_comment_begin >= 0 ? pos_comment_begin : line->Length;
							string_append_sub(&item->Value, line->Data, line->Length, pos_value_end +1, b - pos_value_end -1);
						}

						yason_element_array_add(&current->Children, item);
					}
				}
				iz++;
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






