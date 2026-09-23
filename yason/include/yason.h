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

	// Modo de ligacao DESTA biblioteca. Estatico e o padrao.
	//   YASON_BUILD_SHARED -> compilando o yason como biblioteca compartilhada
	//   YASON_USE_SHARED   -> consumindo o yason compartilhado
	#if defined(_WIN32) || defined(_WIN64)
	    #if   defined(YASON_BUILD_SHARED)
	        #define YASON_API __declspec ( dllexport )
	    #elif defined(YASON_USE_SHARED)
	        #define YASON_API __declspec ( dllimport )
	    #else
	        #define YASON_API
	    #endif
	#else
	    #if defined(YASON_BUILD_SHARED)
	        #define YASON_API __attribute__ ( ( visibility ( "default" ) ) )
	    #else
	        #define YASON_API
	    #endif
	#endif


    #include "../src/yason_element.h"
    #include "../submodules/xplatbase/Xplatbase/Xplatbase/include/xplatbase.h"


    YASON_API Element* yason_parse(const char* content, int length, TreeTypeOption type);
    YASON_API Element* yason_parse_file(const char* path_file);
    YASON_API StringX* yason_render(Element* root, int indent);
    YASON_API void yason_render_file(Element* root, int indent, const char* path_file);


#ifdef __cplusplus
}
#endif

#endif /* YASON */