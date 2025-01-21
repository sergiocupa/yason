#include "../../yason/yason/include/yason.h"



int main()
{
    //const char* FILE_2 = "E:/git/libs/yason/yasontester/Base-RCNN-FPN.yaml";
    const char* FILE_2 = "E:/git/libs/yason/yasontester/vehicle_yolov4-tiny.cfg";
    //const char* FILE_2 = "E:/git/yason/yason/json_example.json";
    //const char* FILE_2 = "E:/git/syaml/test4.yaml";
    //const char* FILE_2 = "E:/git/syaml/test2.yaml";
    //const char* FILE_2 = "E:/git/syaml/test.yaml";

    // TEstar parse de lista inline e block....

    Element* node = yason_parse_file(FILE_2);

    String* render = yason_render(node, 1);

    return 0;
}
