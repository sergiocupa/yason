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


#ifndef YAML_RENDER_H
#define YAML_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "yason_element.h"
    #include "stringlib.h"


	static const char* yaml_create_pad(int pad)
	{
		char* result = (char*)malloc(sizeof(char) * (pad+1));
		int ix = 0;
		while (ix < pad)
		{
			result[ix] = ' ';
			ix++;
		}
		result[ix] = 0;
		return result;
	}


	static void yaml_render_token(Element* root, String* content, int level)
	{
		int ix = 0;
		while (ix < root->Children.Count)
		{
			Element* n = root->Children.Items[ix];

			if (n->Name.Length > 0)
			{
				if (level > 0)
				{
					const char* pad = yaml_create_pad(level);
					string_append(content, pad);
				}

				string_append(content, n->Name.Data);
				string_append(content, ":");
			}

			if (n->Type == NODE_TYPE_SCALAR)
			{
				if (n->Value.Length > 0)
				{
					if (n->IsString) string_append(content, "\"");
					string_append(content, n->Value.Data);
					if (n->IsString) string_append(content, "\"");
				}

				if (root->Type == NODE_TYPE_SEQUENCE || root->Type == NODE_TYPE_MAP)
				{
					if ((ix + 1) < root->Children.Count)
					{
						string_append(content, ",");
					}
				}
				else
				{
					string_append(content, "\n");
				}

				yaml_render_token(n, content, level + 2);
			}
			else if (n->Type == NODE_TYPE_SEQUENCE || n->Type == NODE_TYPE_MAP)
			{
				int al = (n->InLine) ? 0 : level;

				char* tk1 = n->Type == NODE_TYPE_SEQUENCE ? "[" : "{";
				char* tk2 = n->Type == NODE_TYPE_SEQUENCE ? "]" : "}";

				if (n->Name.Length <= 0) string_append(content, tk1);

				yaml_render_token(n, content, al);

				if (n->Name.Length <= 0) string_append(content, tk2);


				if ((ix + 1) < root->Children.Count)
				{
					if (n->InLine)
					{
						if (n->Type == root->Type)
						{
							string_append(content, ",");
						}
						else
						{
							string_append(content, "\n");
						}
					}
				}
				else
				{
					if (n->Type != root->Type)
					{
						string_append(content, "\n");
					}
				}
			}

			ix++;
		}
	}


	static String* yaml_render(Element* root)
	{
		String* content = string_new();
		yaml_render_token(root, content, 0);
		return content;
	}



#ifdef __cplusplus
}
#endif

#endif /* YAML_RENDER */