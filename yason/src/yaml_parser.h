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


#ifndef YAML_PARSE_H
#define YAML_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "yason_element.h"
    #include "listlib.h"
    #include "stringlib.h"
   


    #define YAML_TOKEN      ":\"\r\n[{(,]})-#"
    #define YAML_TOKEN_LENG 13


	typedef struct _ContentPartYaml
	{
		char       Token;
		int        Level;
		int        Line;
		String     Content;
	}
	ContentPartYaml;


	static ContentPartYaml* yaml_part_content(char token)
	{
		ContentPartYaml* ma = (ContentPartYaml*)malloc(sizeof(ContentPartYaml));
		memset(ma, 0, sizeof(ContentPartYaml));
		ma->Content.Length = 0;
		ma->Token = token;
		string_init(&ma->Content);
		return ma;
	}


	static Element* yaml_create_node_add_value(Element* parent, int is_string, ContentPartYaml* item)
	{
		Element* cnode = yason_element_new();
		cnode->Level = item->Level;
		cnode->Type = NODE_TYPE_SCALAR;
		cnode->IsString = is_string;
		cnode->Parent = parent;
		cnode->Line = item->Line;

		string_append_sub(&cnode->Value, item->Content.Data, item->Content.Length, 0, item->Content.Length);
		string_trim(&cnode->Value);
		yason_element_array_add(&parent->Children, cnode);
		return cnode;
	}


	static Element* yaml_find_previous_node(int level, Element* node)
	{
		if (node->Level < level)
		{
			return node;
		}
		else
		{
			if (node->Parent)
			{
				return yaml_find_previous_node(level, node->Parent);
			}
			else
			{
				return 0;
			}
		}
	}


	static void yaml_append_value(int is_field, ContentPartYaml* item, Element* node, int is_map)
	{
		if (is_field && string_with_content(&item->Content))// valor do campo sem delemitador
		{
			Element* last = node->Children.Items[node->Children.Count - 1];

			if (last->Line == item->Line)
			{
				last->Type = NODE_TYPE_SCALAR;

				string_append_sub(&last->Value, item->Content.Data, item->Content.Length, 0, item->Content.Length);
				string_trim(&last->Value);

				last->IsMap = is_map;
			}
			else
			{
				// Erro: item sem assinatura de campo tentando inserir valor
			}
		}
	}


	static Element* yaml_parse_array(List* tk, int* index, int is_map)
	{
		Element* root = yason_element_new();
		root->Type = is_map ? NODE_TYPE_MAP : NODE_TYPE_SEQUENCE;

		int ix = (*index);
		while (ix < tk->Count)
		{
			ContentPartYaml* item = (ContentPartYaml*)tk->Items[ix];

			if (item->Token == '[' || item->Token == '{' || item->Token == '(')
			{
				ix++;
				Element* ro = yaml_parse_array(tk, &ix, (item->Token == '{'));
				yason_element_array_add(&root->Children, ro);
				continue;
			}
			else if (item->Token == '"')
			{
				ix++;
				if (ix >= tk->Count) break;

				ContentPartYaml* item2 = (ContentPartYaml*)tk->Items[ix];

				if (item2->Token == '"')
				{
					yaml_create_node_add_value(root, 1, item2);
				}
			}
			else if (item->Token == ',')
			{
				if (string_with_content(&item->Content))
				{
					yaml_create_node_add_value(root, 0, item);
				}
			}
			else if (item->Token == ']' || item->Token == '}' || item->Token == ')')
			{
				if (string_with_content(&item->Content))
				{
					yaml_create_node_add_value(root, 0, item);
				}
				ix++;
				break;
			}
			else
			{
				break;
			}

			ix++;
		}

		int suma = root->Line * root->Children.Count;
		int sumb = 0;
		int iy = 0;
		while (iy < root->Children.Count)
		{
			Element* it = root->Children.Items[iy];
			sumb += it->Line;
			iy++;
		}
		root->InLine = suma == sumb;

		if (root->InLine)
		{
			iy = 0;
			while (iy < root->Children.Count)
			{
				root->Children.Items[iy]->InLine = 1;
				iy++;
			}
		}

		(*index) = ix;
		return root;
	}

	static String* yaml_parse_tokens(List* tk, Element* root)
	{
		Element* node = root;
		int   sequence_token = 0;
		int   minus_level = 0;
		int   is_field = 0;
		int   ix = 0;
		Element* first = yason_element_new();

		while (ix < tk->Count)
		{
			ContentPartYaml* item = (ContentPartYaml*)tk->Items[ix];

			if (item->Token == ':')
			{
				if (item->Content.Length > 0)
				{
					Element* anode = yason_element_new();
					anode->Level = item->Level;
					anode->Line = item->Line;
					anode->Type = NODE_TYPE_SCALAR;
					anode->IsMap = sequence_token;
					sequence_token = 0;

					string_append_sub(&anode->Name, item->Content.Data, item->Content.Length, 0, item->Content.Length);

					Element* last = 0;
					if (node->Children.Count > 0) last = node->Children.Items[node->Children.Count - 1];
					else last = node;

					if (item->Level == last->Level)// mantem nivel
					{
						anode->Parent = node;
						yason_element_array_add(&node->Children, anode);
					}
					else if (item->Level > last->Level)// avancar nivel
					{
						anode->Parent = last;
						yason_element_array_add(&last->Children, anode);
						node = last;
					}
					else if (item->Level < last->Level)// voltar nivel
					{
						Element* back = yaml_find_previous_node(item->Level, node);
						anode->Parent = back;
						yason_element_array_add(&back->Children, anode);
						node = back;
					}

					is_field = 1;
				}
				else
				{
					// error
				}
			}
			else if (item->Token == '"')
			{
				ix++;
				if (ix >= tk->Count) break;

				ContentPartYaml* item2 = (ContentPartYaml*)tk->Items[ix];

				if (item2->Token == '"')
				{
					if (is_field)
					{
						Element* last = node->Children.Items[node->Children.Count - 1];
						last->Type = NODE_TYPE_SCALAR;
						last->IsString = 1;
						string_append_sub(&last->Value, item2->Content.Data, item2->Content.Length, 0, item2->Content.Length);
						string_trim(&last->Value);
					}
					is_field = 0;
				}
				else
				{
					// erro 
				}
			}
			else if (item->Token == '-')
			{
				sequence_token = 1;
			}
			else if (item->Token == '[' || item->Token == '{' || item->Token == '(')
			{
				Element* xnode = node->Children.Items[node->Children.Count - 1];

				Element* enode = yaml_parse_array(tk, &ix, (item->Token == '{'));

				xnode->Type   = enode->Type;
				xnode->InLine = enode->InLine;

				yason_element_array_transfer(&enode->Children, &xnode->Children, 0);
				free(enode);
				continue;
			}
			else if (item->Token == '#')// Comentario
			{
				ix++;
				if (ix >= tk->Count) break;

				ContentPartYaml* item2 = (ContentPartYaml*)tk->Items[ix];

				if ((item2->Token == '\r') || (item2->Token == '\n'))
				{
					yaml_append_value(is_field, item, node, sequence_token);
					sequence_token = 0;

					Element* last = node->Children.Items[node->Children.Count - 1];

					string_init(&last->Comment);
					string_append_sub(&last->Comment, item2->Content.Data, item2->Content.Length, 0, item2->Content.Length);
				}
			}
			else if ((item->Token == '\r') || (item->Token == '\n'))
			{
				yaml_append_value(is_field, item, node, sequence_token);
				sequence_token = 0;

				is_field = 0;
			}

			ix++;
		}
		return 0;
	}


	static Element* yaml_parse_root_tokens(List* tk)
	{
		Element* root  = yason_element_new();
		root->Type     = NODE_TYPE_ROOT;
		root->TreeType = TREE_TYPE_YAML;
		root->Level    = -1;

		yaml_parse_tokens(tk, root);
		return root;
	}


	static List* yaml_split_by_token(const char* content, int lenght)
	{
		List* list = list_create(sizeof(ContentPartYaml));
		int  current_position = 0;
		char last = 0;
		int  pos = 0;
		int  line = 0;

		while (pos < lenght)
		{
			int p = string_index_first(content, lenght, YAML_TOKEN, YAML_TOKEN_LENG, pos, &current_position);

			if (p >= 0)
			{
				ContentPartYaml* ma = yaml_part_content(YAML_TOKEN[p]);
				ma->Line = line;

				int prev_content_count = current_position - pos;
				if (prev_content_count > 0)// add conteudo antes
				{
					if (last == '\r' || last == '\n')
					{
						int max = current_position - pos;
						int lp = 0;
						ma->Level = string_token_count(content, lenght, ' ', pos, max, &lp);
						int po = ma->Level > 0 ? lp : pos;
						int cn = prev_content_count - (po - pos);

						string_append_sub(&ma->Content, content, lenght, po, cn);
					}
					else
					{
						string_append_sub(&ma->Content, content, lenght, pos, current_position - pos);
					}
				}

				list_add(list, ma, sizeof(ContentPartYaml));

				if ((YAML_TOKEN[p] == '\r' || YAML_TOKEN[p] == '\n') && last != '\r')
				{
					line++;
				}

				last = YAML_TOKEN[p];
				pos = current_position + 1;
			}
			else break;
		}

		int last_content_count = lenght - pos;
		if (last_content_count > 0)// add ultimo conteudo
		{
			ContentPartYaml* ma3 = yaml_part_content(0);

			string_append_sub(&ma3->Content, content, lenght, current_position, last_content_count);
			list_add(list, ma3, sizeof(ContentPartYaml));
		}

		return list;
	}


	static Element* yaml_parse(const char* content, int length)
	{

		List*    pcontents = yaml_split_by_token(content, length);
		Element* root      = yaml_parse_root_tokens(pcontents);

		return root;
	}


#ifdef __cplusplus
}
#endif

#endif /* YAML_PARSE */