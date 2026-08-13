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


#include "../include/yason.h"
#include "json_parser.h"
#include "yaml_parser.h"
#include "cfg_parser.h"
#include "yaml_render.h"
#include "json_render.h"
#include "yason_compat.h"
#include <stdlib.h>


static void yaml_render_tree(Element* root, StringX* content, int level)
{
	int ix = 0;
	while (ix < root->Children.Count)
	{
		Element* n = root->Children.Items[ix];

		if (level > 0)
		{
			const char* pad = yaml_create_pad(level);
			yason_string_append(content, pad);
		}

		yason_string_append(content, ">");


		if (n->Name.Length > 0)
		{
			yason_string_append(content, n->Name.Content);
			yason_string_append(content, ":");
		}

		if (n->Value.Length > 0)
		{
			if (n->IsString) yason_string_append(content, "\"");
			yason_string_append(content, n->Value.Content);
			if (n->IsString) yason_string_append(content, "\"");
		}

		yason_string_append(content, "\n");

		yaml_render_tree(n, content, level + 2);

		ix++;
	}


}


static TreeTypeOption yason_get_tree_type(const char* file_name)
{
	int stop = yason_string_index_end_char(file_name, '.');

	if (stop > 0)
	{
		char* upper = yason_string_to_upper_copy(file_name);

		if (yason_string_equals_char_range(upper, "JSON", stop+1, -1))
		{
			memop_free_raw(upper);
			return TREE_TYPE_JSON;
		}
		else if (yason_string_equals_char_range(upper, "YAML", stop+1, -1))
		{
			memop_free_raw(upper);
			return TREE_TYPE_YAML;
		}
		else if (yason_string_equals_char_range(upper, "CFG", stop + 1, -1))
		{
			memop_free_raw(upper);
			return TREE_TYPE_CFG;
		}
		memop_free_raw(upper);
	}
	return TREE_TYPE_UNKNOWN;
}


StringX* yason_render(Element* root, int indent)
{
	if (root->TreeType == TREE_TYPE_JSON)
	{
		return json_render(root, indent);
	}
	else if (root->TreeType == TREE_TYPE_YAML)
	{
		return yaml_render(root);
	}
	else if (root->TreeType == TREE_TYPE_CFG)
	{
		return cfg_render(root);
	}
	return 0;
}

void yason_render_file(Element* root, int indent, const char* path_file)
{
	StringX* content = yason_render(root, indent);

	file_write_text(path_file, content->Content, content->Length);
}


Element* yason_parse(const char* content, int length, TreeTypeOption type)
{
	switch (type)
	{
	case TREE_TYPE_JSON:
		return json_parse(content, length);
	case TREE_TYPE_YAML:
		return yaml_parse(content, length);  
	case TREE_TYPE_CFG:
		return cfg_parse(content, length);
	}
	return 0;
}


Element* yason_parse_file(const char* path_file)
{
	int   length = 0;
	char* content = 0;

	int op = file_read_text(path_file, &content, &length);

	if (op && length > 0)
	{
		TreeTypeOption type = yason_get_tree_type(path_file);

		Element* root = yason_parse(content, length, type);
		return root;
	}

	return 0;
}


Element* yason_find(Element* root, const int recursive, const char* token)
{
	char* up1 = yason_string_to_upper_copy(token);

	int ix = 0;
	while (ix < root->Children.Count)
	{
		Element* ele = root->Children.Items[ix];
		char* up2 = yason_string_to_upper_copy(ele->Name.Content);

		if (yason_string_equals_char(up1, up2))
		{
			memop_free_raw(up1);
			memop_free_raw(up2);
			return ele;
		}

		if (recursive)
		{
			Element* eles = yason_find(ele, recursive, token);
			if (eles)
			{
				memop_free_raw(up1);
				memop_free_raw(up2);
				return eles;
			}
		}

		memop_free_raw(up2);
		ix++;
	}
	memop_free_raw(up1);
	return 0;
}


char* yason_find_string(Element* root, const int recursive, const char* token, const char* _default)
{
	Element* ele = yason_find(root, recursive, token);
	if (ele)
	{
		if (ele->Value.Length)
		{
			return ele->Value.Content;
		}
		else
		{
			return _default;
		}
	}
	else
	{
		return _default;
	}
}

int yason_find_int(Element* root, const int recursive, const char* token, int _default)
{
	Element* ele = yason_find(root, recursive, token);
	if (ele)
	{
		int error = 0;
		int value = numeric_parse_int(ele->Value.Content, &error);

		if (error)
		{
			return value;
		}
		else
		{
			return _default;
		}
	}
	else
	{
		return _default;
	}
}

double yason_find_double(Element* root, const int recursive, const char* token, int _default)
{
	Element* ele = yason_find(root, recursive, token);
	if (ele)
	{
		int    error = 0;
		double value = numeric_parse_double(ele->Value.Content, &error);

		if (error)
		{
			return value;
		}
		else
		{
			return _default;
		}
	}
	else
	{
		return _default;
	}
}