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


#ifndef JSON_RENDER_H
#define JSON_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

	#include "yason_element.h"
    #include "stringlib.h"


    #define INDENT_STEP 2

	static void json_render_object(Element* obj, String* content, int indent, int is_root);


	void json_indent(String* content, const int indent, const int append_line)
	{
		if (indent >= 0)
		{
			if (append_line)
			{
				string_append_char(content, '\n');
			}

			int ix = 0;
			while (ix < indent)
			{
				string_append_char(content, ' ');
				ix++;
			}
		}
	}

	static void json_append_value(Element* field, String* content)
	{
		if (field->IsString)
		{
			string_append(content, "\"");
			string_append(content, field->Value.Data);
			string_append(content, "\"");
		}
		else
		{
			string_append(content, field->Value.Data);
		}
	}

	static void json_append_field(Element* field, String* content)
	{
		string_append(content, "\"");
		string_append(content, field->Name.Data);
		string_append(content, "\":");

		if (field->IsString)
		{
			string_append(content, "\"");

			if (field->Value.Length > 0) string_append(content, field->Value.Data);

			string_append(content, "\"");
		}
		else
		{
			if(field->Value.Length > 0) string_append(content, field->Value.Data);
		}
	}


	static void json_render_array(Element* ary, String* content, int indent)
	{
		if (ary->Children.Count > 0)
		{
			int op = indent;
			if (indent >= 0)
			{
				json_indent(content, indent, 1);
			}

			string_append(content, "[");

			if (indent >= 0)
			{
				indent += INDENT_STEP;
				json_indent(content, indent, 1);
			}

			int CNT = ary->Children.Count - 1;
			int ix = 0;
			while (ix < CNT)
			{
				Element* element = ary->Children.Items[ix];

				if (element->Type == NODE_TYPE_OBJECT)
				{
					json_render_object(element, content, indent, 0);
				}
				else
				{
					json_append_value(element, content);
				}

				string_append(content, ",");

				if (indent >= 0) json_indent(content, indent, 1);
				ix++;
			}


			Element* last = ary->Children.Items[ix];

			if (last->Type == NODE_TYPE_OBJECT)
			{
				json_render_object(last, content, indent, 0);
			}
			else
			{
				json_append_value(last, content);
			}

			if (op >= 0) json_indent(content, op, 1);


			string_append(content, "]");
		}
	}


	static void json_render_object(Element* obj, String* content, int indent, int is_root)
	{
		if (obj->Children.Count > 0)
		{
			int op = indent;
			if (indent >= 0)
			{
				json_indent(content, indent, !is_root);
			}

			string_append(content, "{");

			if (indent >= 0)
			{
				indent += INDENT_STEP;
				json_indent(content, indent, 1);
			}

			int CNT = obj->Children.Count - 1;
			int ix = 0;
			while (ix < CNT)
			{
				Element* field = obj->Children.Items[ix];

				json_append_field(field, content);

				if (field->Type == NODE_TYPE_OBJECT)
				{
					json_render_object(field, content, indent, 0);
				}
				else if (field->Type == NODE_TYPE_ARRAY)
				{
					json_render_array(field, content, indent);
				}

				string_append(content, ",");

				if (indent >= 0) json_indent(content, indent, 1);

				ix++;
			}

			Element* last = obj->Children.Items[ix];
			json_append_field(last, content);

			if (last->Type == NODE_TYPE_OBJECT)
			{
				json_render_object(last, content, indent, 0);
			}
			else if (last->Type == NODE_TYPE_ARRAY)
			{
				json_render_array(last, content, indent);
			}

			if (op >= 0) json_indent(content, op, 1);

			string_append(content, "}");
		}
	}


	static String* json_render(Element* root, int indent)
	{
		String* content = string_new();

		indent = indent > 0 ? 0 : -1;

		if (root->Type = NODE_TYPE_OBJECT)
		{
			json_render_object(root, content, indent, 1);
			
		}
		else
		{
			json_render_array(root, content, indent);
		}

		return content;
	}


#ifdef __cplusplus
}
#endif

#endif /* JSON_RENDER */