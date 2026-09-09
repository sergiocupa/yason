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
	#include "yason_compat.h"
    #include <string.h>
    #include <stdlib.h>

	typedef struct JsonTokenContent
	{
		char   Token;
		StringX Content;
	}
	JsonTokenContent;

	static Element* json_parse_object(ListX* elements, int* index);


	/* Move o texto acumulado para um token novo e reinicia o acumulador. A posse do
	 * buffer e transferida (copia da struct), sem realocar. */
	static void json_push_token(ListX* list, char token, StringX* pending)
	{
		JsonTokenContent* ma = (JsonTokenContent*)memop_alloc_raw(sizeof(JsonTokenContent));
		memset(ma, 0, sizeof(JsonTokenContent));

		ma->Token   = token;
		ma->Content = *pending;

		list_add(list, ma, sizeof(JsonTokenContent));

		string_init(pending);
	}


	/* Acrescenta um code point como UTF-8 (usado na decodificacao de \uXXXX). */
	static void json_append_utf8(StringX* dst, unsigned int cp)
	{
		if (cp < 0x80)
		{
			yason_string_append_char(dst, (char)cp);
		}
		else if (cp < 0x800)
		{
			yason_string_append_char(dst, (char)(0xC0 | (cp >> 6)));
			yason_string_append_char(dst, (char)(0x80 | (cp & 0x3F)));
		}
		else if (cp < 0x10000)
		{
			yason_string_append_char(dst, (char)(0xE0 | (cp >> 12)));
			yason_string_append_char(dst, (char)(0x80 | ((cp >> 6) & 0x3F)));
			yason_string_append_char(dst, (char)(0x80 | (cp & 0x3F)));
		}
		else
		{
			yason_string_append_char(dst, (char)(0xF0 | (cp >> 18)));
			yason_string_append_char(dst, (char)(0x80 | ((cp >> 12) & 0x3F)));
			yason_string_append_char(dst, (char)(0x80 | ((cp >> 6) & 0x3F)));
			yason_string_append_char(dst, (char)(0x80 | (cp & 0x3F)));
		}
	}


	/* Le 4 digitos hexadecimais a partir de 'pos'. 1 = ok. */
	static int json_hex4(const char* s, int length, int pos, unsigned int* out)
	{
		unsigned int v = 0;
		int i;

		if (pos + 4 > length) return 0;

		for (i = 0; i < 4; i++)
		{
			char c = s[pos + i];
			v <<= 4;
			if      (c >= '0' && c <= '9') v |= (unsigned int)(c - '0');
			else if (c >= 'a' && c <= 'f') v |= (unsigned int)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F') v |= (unsigned int)(c - 'A' + 10);
			else return 0;
		}

		*out = v;
		return 1;
	}


	/* Tokenizador CIENTE DE ASPAS.
	 *
	 * A versao anterior procurava qualquer um de ":,{}[]" sem saber se estava dentro de
	 * uma string. Com isso um valor como "2026-09-09T02:00:11Z" gerava tokens ':' no meio,
	 * quebrava o padrao que json_parse_object espera e o CAMPO INTEIRO desaparecia na
	 * leitura -- silenciosamente. Aqui os delimitadores so contam FORA de string, e as
	 * sequencias de escape sao decodificadas para o conteudo real do token.
	 *
	 * O formato dos tokens e o mesmo de antes (Content = texto entre o delimitador
	 * anterior e este; o conteudo de uma string fica no token da aspa que a FECHA),
	 * entao os parsers de objeto/array continuam valendo sem alteracao. */
	static ListX* json_index_tokens(const char* content, const int length)
	{
		ListX*  list      = yason_list_create(sizeof(JsonTokenContent));
		int     in_string = 0;
		int     ix        = 0;
		StringX pending;

		string_init(&pending);

		while (ix < length)
		{
			char c = content[ix];

			if (in_string)
			{
				if (c == '\\' && ix + 1 < length)
				{
					char e = content[ix + 1];
					ix += 2;

					switch (e)
					{
						case '"':  yason_string_append_char(&pending, '"');  break;
						case '\\': yason_string_append_char(&pending, '\\'); break;
						case '/':  yason_string_append_char(&pending, '/');  break;
						case 'b':  yason_string_append_char(&pending, '\b'); break;
						case 'f':  yason_string_append_char(&pending, '\f'); break;
						case 'n':  yason_string_append_char(&pending, '\n'); break;
						case 'r':  yason_string_append_char(&pending, '\r'); break;
						case 't':  yason_string_append_char(&pending, '\t'); break;
						case 'u':
						{
							unsigned int cp = 0;
							if (json_hex4(content, length, ix, &cp))
							{
								ix += 4;

								/* par surrogate: \uD800-\uDBFF seguido de \uDC00-\uDFFF */
								if (cp >= 0xD800 && cp <= 0xDBFF && ix + 6 <= length &&
									content[ix] == '\\' && content[ix + 1] == 'u')
								{
									unsigned int lo = 0;
									if (json_hex4(content, length, ix + 2, &lo) && lo >= 0xDC00 && lo <= 0xDFFF)
									{
										cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
										ix += 6;
									}
								}

								json_append_utf8(&pending, cp);
							}
							break;
						}
						default: yason_string_append_char(&pending, e); break;
					}
					continue;
				}

				if (c == '"')   /* fecha a string: o conteudo acumulado vai neste token */
				{
					json_push_token(list, '"', &pending);
					in_string = 0;
					ix++;
					continue;
				}

				yason_string_append_char(&pending, c);
				ix++;
				continue;
			}

			if (c == '"')       /* abre a string */
			{
				json_push_token(list, '"', &pending);
				in_string = 1;
				ix++;
				continue;
			}

			if (c == ':' || c == ',' || c == '{' || c == '}' || c == '[' || c == ']')
			{
				json_push_token(list, c, &pending);
				ix++;
				continue;
			}

			yason_string_append_char(&pending, c);
			ix++;
		}

		string_release(&pending);   /* sobra apos o ultimo delimitador: nao vira token */

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
			yason_string_append_sub(&field->Name, name_element->Content.Content, name_element->Content.Length, 0, name_element->Content.Length);
			string_trim(&field->Name);
		}

		if (value_element)
		{
			yason_string_append_sub(&field->Value, value_element->Content.Content, value_element->Content.Length, 0, value_element->Content.Length);
		}
		
		string_trim(&field->Value);
		yason_element_array_add(&obj->Children, field);
	}

	static Element* json_parse_array(ListX* elements, int* index)
	{
		Element* arra  = yason_element_new();
		arra->TreeType = TREE_TYPE_JSON;
		arra->Type     = NODE_TYPE_ARRAY;

		int last_value = 0;

		int ix = (*index);
		while (ix < elements->Count)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Items[ix];

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
				JsonTokenContent* element2 = (JsonTokenContent*)elements->Items[ix];

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
				/* O valor NAO-string de um item fica no Content da propria virgula
				 * ([1,2,3] -> tokens ',' com "1" e "2"). Depois de um item que JA foi
				 * criado (string ou objeto), a virgula vem vazia e criava um campo
				 * fantasma -- ["a","b"] virava ["a",,"b",,]. Mesma guarda do ramo ']'. */
				if (!last_value && yason_string_with_content(&element->Content))
				{
					json_create_field(arra, 0, element, 0);
				}
				last_value = 0;
			}
			else if (element->Token == ']')
			{
				if (!last_value && yason_string_with_content(&element->Content))
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

	static Element* json_parse_object(ListX* elements, int* index)
	{
		Element* obj  = yason_element_new();
		obj->TreeType = TREE_TYPE_JSON;
		obj->Type     = NODE_TYPE_OBJECT;

		int ix = (*index);
		while (ix < elements->Count)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Items[ix];

			if (element->Token == '"')
			{
				ix++;
				JsonTokenContent* element2 = (JsonTokenContent*)elements->Items[ix];

				if (element2->Token == '"')
				{
					ix++;
					JsonTokenContent* element3 = (JsonTokenContent*)elements->Items[ix];

					if (element3->Token == ':')
					{
						ix++;
						JsonTokenContent* element4 = (JsonTokenContent*)elements->Items[ix];

						if (element4->Token == '"')// valor string
						{
							ix++;
							JsonTokenContent* element5 = (JsonTokenContent*)elements->Items[ix];

							if (element5->Token == '"')
							{
								ix++;
								JsonTokenContent* element6 = (JsonTokenContent*)elements->Items[ix];

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

							yason_string_append_sub(&ar->Name, element2->Content.Content, element2->Content.Length, 0, element2->Content.Length);
							string_trim(&ar->Name);

							yason_element_array_add(&obj->Children, ar);
							continue;
						}
						else if (element4->Token == '{')// campo objeto
						{
							ix++;
							Element* no = json_parse_object(elements, &ix);

							yason_string_append_sub(&no->Name, element2->Content.Content, element2->Content.Length, 0, element2->Content.Length);
							string_trim(&no->Name);

							yason_element_array_add(&obj->Children, no);
							continue;
						}
						else if (element4->Token == '}')// valor NAO-string no fim do objeto
						{
							/* {"a":1} -- o valor fica no Content do proprio '}'. Antes este
							 * ramo so fechava o objeto e o campo era DESCARTADO na leitura. */
							if (yason_string_with_content(&element4->Content))
							{
								json_create_field(obj, element2, element4, 0);
							}
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


	static Element* json_parse_elements(ListX* elements)
	{
		Element* root = 0;

		int ix = 0;
		while (ix < elements->Count)
		{
			JsonTokenContent* element = (JsonTokenContent*)elements->Items[ix];

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
		ListX*    elements = json_index_tokens(content, length);
		Element* root     = json_parse_elements(elements);
		return root;
	}

#ifdef __cplusplus
}
#endif

#endif /* JSON_PARSER */