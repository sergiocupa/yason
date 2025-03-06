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