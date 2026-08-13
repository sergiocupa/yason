#include "../../yason/yason/include/yason.h"
#include <stdio.h>

static int tree_string_equal(const StringX* left, const StringX* right)
{
    uint64 index;
    if (left->Length != right->Length) return 0;
    for (index = 0; index < left->Length; ++index)
    {
        if (left->Content[index] != right->Content[index]) return 0;
    }
    return 1;
}

static int element_equal(const Element* left, const Element* right, const char* path)
{
    int index;
    char child_path[512];

    if (!left || !right)
    {
        printf("FALHA em %s: um dos nos e nulo.\n", path);
        return 0;
    }
    if (left->Type != right->Type || left->IsString != right->IsString ||
        !tree_string_equal(&left->Name, &right->Name) ||
        !tree_string_equal(&left->Value, &right->Value) ||
        left->Children.Count != right->Children.Count)
    {
        printf("FALHA em %s: estrutura divergente.\n", path);
        printf("  original: tipo=%d nome='%s' valor='%s' string=%d filhos=%d\n",
            left->Type, left->Name.Content, left->Value.Content,
            left->IsString, left->Children.Count);
        printf("  restaurado: tipo=%d nome='%s' valor='%s' string=%d filhos=%d\n",
            right->Type, right->Name.Content, right->Value.Content,
            right->IsString, right->Children.Count);
        return 0;
    }

    for (index = 0; index < left->Children.Count; ++index)
    {
        sprintf_s(child_path, sizeof(child_path), "%s/%d", path, index);
        if (!element_equal(left->Children.Items[index], right->Children.Items[index], child_path)) return 0;
    }
    return 1;
}

int main(int argc, char** argv)
{
    const char* file = argc > 1 ? argv[1] : "vehicle_yolov4-tiny.cfg";// "E:/git/libs/yason/yasontester/vehicle_yolov4-tiny.cfg";
    Element* original = yason_parse_file(file);
    StringX* rendered;
    Element* restored;

    if (!original) { printf("FALHA: nao foi possivel analisar '%s'.\n", file); return 1; }
    rendered = yason_render(original, 1);
    if (!rendered || !rendered->Content || rendered->Length == 0)
    {
        printf("FALHA: a renderizacao ficou vazia.\n");
        return 2;
    }
    restored = yason_parse(rendered->Content, (int)rendered->Length, original->TreeType);
    if (!restored) { printf("FALHA: nao foi possivel analisar o texto renderizado.\n"); return 3; }
    if (!element_equal(original, restored, "raiz")) return 4;

    printf("OK: conversao restaurou a estrutura (%d secoes na raiz, %llu bytes renderizados).\n",
        original->Children.Count, (unsigned long long)rendered->Length);
    return 0;
}
