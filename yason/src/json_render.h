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
    #include "yason_compat.h"


    #define INDENT_STEP 2

	static void json_render_object(Element* obj, StringX* content, int indent, int is_root);


	void json_indent(StringX* content, const int indent, const int append_line)
	{
		if (indent >= 0)
		{
			if (append_line)
			{
				yason_string_append_char(content, '\n');
			}

			int ix = 0;
			while (ix < indent)
			{
				yason_string_append_char(content, ' ');
				ix++;
			}
		}
	}

	/* Escapa uma string para JSON. O render anterior copiava o valor CRU entre aspas:
	 * uma aspa, uma barra invertida ou um caractere de controle no valor produziam JSON
	 * invalido (um caminho do Windows como "\\\\?\\usb#..." quebrava o JSON.parse do browser).
	 * E a contrapartida da decodificacao de escapes feita no tokenizador. */
	static void json_append_escaped(StringX* content, const char* value, int value_length)
	{
		int i;

		if (!value) return;

		for (i = 0; i < value_length; i++)
		{
			unsigned char c = (unsigned char)value[i];

			switch (c)
			{
				case '"':  yason_string_append(content, "\\\""); break;
				case '\\': yason_string_append(content, "\\\\"); break;
				case '\b': yason_string_append(content, "\\b");  break;
				case '\f': yason_string_append(content, "\\f");  break;
				case '\n': yason_string_append(content, "\\n");  break;
				case '\r': yason_string_append(content, "\\r");  break;
				case '\t': yason_string_append(content, "\\t");  break;
				default:
					if (c < 0x20)
					{
						char u[8];
						snprintf(u, sizeof(u), "\\u%04X", (unsigned int)c);
						yason_string_append(content, u);
					}
					else
					{
						yason_string_append_char(content, (char)c);
					}
					break;
			}
		}
	}


	static void json_append_value(Element* field, StringX* content)
	{
		if (field->IsString)
		{
			yason_string_append(content, "\"");
			json_append_escaped(content, field->Value.Content, field->Value.Length);
			yason_string_append(content, "\"");
		}
		else
		{
			/* Um escalar nao-string vazio geraria um vazio sintatico. */
			if (field->Value.Length > 0) yason_string_append(content, field->Value.Content);
			else                         yason_string_append(content, "null");
		}
	}

	static void json_append_field(Element* field, StringX* content)
	{
		yason_string_append(content, "\"");
		json_append_escaped(content, field->Name.Content, field->Name.Length);
		yason_string_append(content, "\":");

		/* Objeto/array: o valor e escrito pelo render aninhado, logo depois. */
		if (field->Type == NODE_TYPE_OBJECT || field->Type == NODE_TYPE_ARRAY) return;

		if (field->IsString)
		{
			yason_string_append(content, "\"");
			json_append_escaped(content, field->Value.Content, field->Value.Length);
			yason_string_append(content, "\"");
		}
		else
		{
			/* Sem isto, um campo nao-string vazio saia como `"nome":` -- JSON invalido. */
			if (field->Value.Length > 0) yason_string_append(content, field->Value.Content);
			else                         yason_string_append(content, "null");
		}
	}


	static void json_render_array(Element* ary, StringX* content, int indent)
	{
		/* Array VAZIO nao escrevia nada, produzindo `"tracks":` -- JSON invalido. */
		if (ary->Children.Count == 0)
		{
			yason_string_append(content, "[]");
			return;
		}

		{
			int op = indent;
			if (indent >= 0)
			{
				json_indent(content, indent, 1);
			}

			yason_string_append(content, "[");

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
				else if (element->Type == NODE_TYPE_ARRAY)
				{
					json_render_array(element, content, indent);
				}
				else
				{
					json_append_value(element, content);
				}

				yason_string_append(content, ",");

				if (indent >= 0) json_indent(content, indent, 1);
				ix++;
			}


			Element* last = ary->Children.Items[ix];

			if (last->Type == NODE_TYPE_OBJECT)
			{
				json_render_object(last, content, indent, 0);
			}
			else if (last->Type == NODE_TYPE_ARRAY)
			{
				json_render_array(last, content, indent);
			}
			else
			{
				json_append_value(last, content);
			}

			if (op >= 0) json_indent(content, op, 1);


			yason_string_append(content, "]");
		}
	}


	static void json_render_object(Element* obj, StringX* content, int indent, int is_root)
	{
		/* Mesmo caso do array vazio: antes nao se escrevia nada. */
		if (obj->Children.Count == 0)
		{
			yason_string_append(content, "{}");
			return;
		}

		{
			int op = indent;
			if (indent >= 0)
			{
				json_indent(content, indent, !is_root);
			}

			yason_string_append(content, "{");

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

				yason_string_append(content, ",");

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

			yason_string_append(content, "}");
		}
	}


	static StringX* json_render(Element* root, int indent)
	{
		StringX* content = yason_string_new();

		indent = indent > 0 ? 0 : -1;

		/* Era uma ATRIBUICAO (`=`), nao uma comparacao: o no raiz era sempre forcado a
		 * objeto e um array na raiz nunca era renderizado como array. */
		if (root->Type == NODE_TYPE_ARRAY)
		{
			json_render_array(root, content, indent);
		}
		else
		{
			json_render_object(root, content, indent, 1);
		}

		return content;
	}


#ifdef __cplusplus
}
#endif

#endif /* JSON_RENDER */