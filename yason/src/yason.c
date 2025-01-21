#include "../include/yason.h"
#include "json_parser.h"
#include "yaml_parser.h"
#include "cfg_parser.h"
#include "yaml_render.h"
#include "json_render.h"
#include "../../../filelib/include/filelib.h"
#include "../../../numericlib/include/numeric.h"
#include "../../../stringlib/include/stringlib.h"
#include <stdlib.h>


static void yaml_render_tree(Element* root, String* content, int level)
{
	int ix = 0;
	while (ix < root->Children.Count)
	{
		Element* n = root->Children.Items[ix];

		if (level > 0)
		{
			const char* pad = yaml_create_pad(level);
			string_append(content, pad);
		}

		string_append(content, ">");


		if (n->Name.Length > 0)
		{
			string_append(content, n->Name.Data);
			string_append(content, ":");
		}

		if (n->Value.Length > 0)
		{
			if (n->IsString) string_append(content, "\"");
			string_append(content, n->Value.Data);
			if (n->IsString) string_append(content, "\"");
		}

		string_append(content, "\n");

		yaml_render_tree(n, content, level + 2);

		ix++;
	}


}


static TreeTypeOption yason_get_tree_type(const char* file_name)
{
	int stop = string_index_end_char(file_name, '.');

	if (stop > 0)
	{
		char* upper = string_to_upper_copy_achar(file_name);

		if (string_equals_char_range(upper, "JSON", stop+1, -1))
		{
			free(upper);
			return TREE_TYPE_JSON;
		}
		else if (string_equals_char_range(upper, "YAML", stop+1, -1))
		{
			free(upper);
			return TREE_TYPE_YAML;
		}
		else if (string_equals_char_range(upper, "CFG", stop + 1, -1))
		{
			free(upper);
			return TREE_TYPE_CFG;
		}
		free(upper);
	}
	return TREE_TYPE_UNKNOWN;
}


String* yason_render(Element* root, int indent)
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
	String* content = yason_render(root, indent);

	file_write_text(path_file, content->Data, content->Length);
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
	char* up1 = string_to_upper_copy_achar(token);

	int ix = 0;
	while (ix < root->Children.Count)
	{
		Element* ele = root->Children.Items[ix];
		char* up2 = string_to_upper_copy_achar(&ele->Name.Data);

		if (string_equals_char(up1, up2))
		{
			free(up1);
			free(up2);
			return ele;
		}

		if (recursive)
		{
			Element* eles = yason_find(ele, recursive, token);
			if (eles)
			{
				free(up1);
				free(up2);
				return eles;
			}
		}

		free(up2);
		ix++;
	}
	free(up1);
	return 0;
}


char* yason_find_string(Element* root, const int recursive, const char* token, const char* _default)
{
	Element* ele = yason_find(root, recursive, token);
	if (ele)
	{
		if (ele->Value.Length)
		{
			return ele->Value.Data;
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
		int value = numeric_parse_int(ele->Value.Data, &error);

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
		double value = numeric_parse_double(ele->Value.Data, &error);

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