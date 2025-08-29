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


#ifndef YASON_H
#define YASON_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "../src/yason_element.h"
    #include "platform.h"


    PLATFORM_API Element* yason_parse(const char* content, int length, TreeTypeOption type);
    PLATFORM_API Element* yason_parse_file(const char* path_file);
    PLATFORM_API String* yason_render(Element* root, int indent);
    PLATFORM_API void yason_render_file(Element* root, int indent, const char* path_file);


#ifdef __cplusplus
}
#endif

#endif /* YASON */