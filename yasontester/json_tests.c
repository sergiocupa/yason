//  MIT License - Modified for Mandatory Attribution
//  Copyright(c) 2025 Sergio Paludo - github.com/sergiocupa
//
//  Testes de regressao do caminho JSON.
//
//  Cada bloco cobre um defeito que ja esteve presente e foi corrigido. Sao todos casos
//  reais, encontrados usando a lib num servico HTTP: eles falhavam SILENCIOSAMENTE --
//  um campo sumia da arvore, ou a saida virava JSON invalido que so quebrava no cliente.
//  Por isso a checagem e sempre por comparacao de texto/valor, nunca por "nao crashou".

#include "json_tests.h"

#include "../yason/include/yason.h"
#include "../yason/src/yason_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

/* ---- infraestrutura minima ------------------------------------------------ */

static void report(int ok, const char* group, const char* what, const char* got, const char* want)
{
    if (ok) { g_pass++; printf("  [ok   ] %-14s %s\n", group, what); return; }

    g_fail++;
    printf("  [FALHA] %-14s %s\n", group, what);
    printf("           obtido  : %s\n", got  ? got  : "(nulo)");
    printf("           esperado: %s\n", want ? want : "(nulo)");
}

static void expect_text(const char* group, const char* what, const char* got, const char* want)
{
    int ok = (got && want) ? (strcmp(got, want) == 0) : (got == want);
    report(ok, group, what, got, want);
}

/* parse -> render, devolvendo o texto renderizado (ou NULL). */
static const char* round_trip(const char* json)
{
    Element* tree = yason_parse(json, (int)strlen(json), TREE_TYPE_JSON);
    StringX* out;

    if (!tree) return 0;

    out = yason_render(tree, 0);
    return (out && out->Content) ? out->Content : 0;
}

/* Valor de um campo pelo nome (NULL se o campo nao existe na arvore). */
static const char* field_value(Element* obj, const char* name)
{
    Element* e = obj ? yason_find_element(obj, name) : 0;
    if (!e) return 0;
    return e->Value.Content ? e->Value.Content : "";
}

static Element* parse_json(const char* json)
{
    return yason_parse(json, (int)strlen(json), TREE_TYPE_JSON);
}

/* Cria um campo escalar dentro de um objeto/array ja existente. */
static Element* make_child(Element* parent, const char* name, ElementType type)
{
    Element* e = yason_element_new();
    e->Type     = type;
    e->TreeType = TREE_TYPE_JSON;
    e->Parent   = parent;
    if (name) yason_string_append(&e->Name, name);
    yason_element_array_add(&parent->Children, e);
    return e;
}

static Element* make_root_object(void)
{
    Element* e = yason_element_new();
    e->Type     = NODE_TYPE_OBJECT;
    e->TreeType = TREE_TYPE_JSON;
    return e;
}

/* ---- os testes ------------------------------------------------------------ */

/* O tokenizador procurava ":,{}[]" sem saber se estava dentro de uma string. Um valor
 * comum como um timestamp ISO gerava tokens ':' no meio, quebrava o padrao esperado pelo
 * parser de objeto e o CAMPO INTEIRO desaparecia -- sem erro, sem aviso. */
static void test_delimitadores_dentro_de_string(void)
{
    const char* group = "delimitador";
    Element* t = parse_json(
        "{\"created\":\"2026-09-09T02:00:11Z\","
        "\"texto\":\"a, b: c {d} [e]\","
        "\"depois\":\"ainda aqui\"}");

    expect_text(group, "timestamp com ':'",      field_value(t, "created"), "2026-09-09T02:00:11Z");
    expect_text(group, "virgula/chaves/colchete", field_value(t, "texto"),   "a, b: c {d} [e]");
    expect_text(group, "campo seguinte intacto",  field_value(t, "depois"),  "ainda aqui");
}

/* O ramo que fechava o objeto tinha apenas um "TO-DO": um valor NAO-string na ultima
 * posicao era descartado. Como um arquivo de estado relido a cada edicao passa por
 * parse/render varias vezes, os campos numericos sumiam um a um, do fim para o inicio. */
static void test_valor_nao_string_no_fim_do_objeto(void)
{
    const char* group = "fim-objeto";
    Element* t = parse_json("{\"a\":1,\"b\":2,\"c\":true,\"ultimo\":42}");

    expect_text(group, "primeiro",        field_value(t, "a"),      "1");
    expect_text(group, "meio",            field_value(t, "b"),      "2");
    expect_text(group, "booleano",        field_value(t, "c"),      "true");
    expect_text(group, "ULTIMO (numero)", field_value(t, "ultimo"), "42");

    /* Duas voltas: era assim que o campo sumia na pratica. */
    {
        const char* volta1 = round_trip("{\"a\":\"x\",\"n\":7}");
        const char* volta2 = volta1 ? round_trip(volta1) : 0;
        expect_text(group, "sobrevive a 2 voltas", volta2, "{\"a\":\"x\",\"n\":7}");
    }
}

/* Array e objeto vazios nao emitiam NADA, produzindo `"tracks":` -- JSON invalido, que
 * so estourava no cliente. */
static void test_containers_vazios(void)
{
    const char* group = "vazio";
    Element* root = make_root_object();

    make_child(root, "tracks", NODE_TYPE_ARRAY);
    make_child(root, "meta",   NODE_TYPE_OBJECT);

    {
        StringX* out = yason_render(root, 0);
        expect_text(group, "array e objeto vazios", out ? out->Content : 0, "{\"tracks\":[],\"meta\":{}}");
    }

    expect_text(group, "array vazio isolado",  round_trip("[]"), "[]");
    expect_text(group, "objeto vazio isolado", round_trip("{}"), "{}");
}

/* json_render usava '=' no lugar de '==' na checagem do tipo da raiz: o no raiz era
 * sempre forcado a objeto e um array na raiz nunca era renderizado como array. */
static void test_array_na_raiz(void)
{
    const char* group = "raiz-array";
    expect_text(group, "strings", round_trip("[\"a\",\"b\",\"c\"]"), "[\"a\",\"b\",\"c\"]");
    expect_text(group, "numeros", round_trip("[1,2,3]"),             "[1,2,3]");
}

/* O render copiava o valor CRU entre aspas. Uma aspa, uma barra invertida ou um
 * caractere de controle produziam JSON invalido -- um caminho de dispositivo do Windows
 * quebrava o JSON.parse do browser. O parse tem que desfazer o que o render fez. */
static void test_escapes(void)
{
    const char* group = "escape";

    expect_text(group, "aspas no valor",   round_trip("{\"a\":\"diz \\\"oi\\\"\"}"), "{\"a\":\"diz \\\"oi\\\"\"}");
    expect_text(group, "barra invertida",  round_trip("{\"a\":\"c:\\\\tmp\"}"),      "{\"a\":\"c:\\\\tmp\"}");
    expect_text(group, "newline e tab",    round_trip("{\"a\":\"l1\\nl2\\tfim\"}"),  "{\"a\":\"l1\\nl2\\tfim\"}");
    expect_text(group, "controle -> \\u",  round_trip("{\"a\":\"\\u0001\"}"),        "{\"a\":\"\\u0001\"}");
    expect_text(group, "aspas na CHAVE",   round_trip("{\"a\\\"b\":1}"),             "{\"a\\\"b\":1}");

    /* Caminho de dispositivo do Windows: o valor tem que voltar EXATAMENTE igual. */
    {
        Element* root = make_root_object();
        Element* f    = make_child(root, "id", NODE_TYPE_FIELD);
        const char* caminho = "\\\\?\\usb#vid_0c45&pid_64ab\\global";
        StringX* out;
        Element* devolta;

        f->IsString = 1;
        yason_string_append(&f->Value, caminho);

        out     = yason_render(root, 0);
        devolta = out ? parse_json(out->Content) : 0;

        expect_text(group, "caminho do Windows", field_value(devolta, "id"), caminho);
    }
}

/* A virgula de array criava um campo a partir do proprio token; depois de um item que ja
 * havia sido criado (string ou objeto) ela vinha vazia e virava um item fantasma:
 * ["a","b"] renderizava como ["a",,"b",,]. */
static void test_itens_fantasma_em_array(void)
{
    const char* group = "array-item";
    expect_text(group, "strings",          round_trip("[\"a\",\"b\"]"),           "[\"a\",\"b\"]");
    expect_text(group, "objetos",          round_trip("[{\"a\":1},{\"a\":2}]"),   "[{\"a\":1},{\"a\":2}]");
    expect_text(group, "um numero so",     round_trip("[7]"),                     "[7]");
    expect_text(group, "bool e null",      round_trip("[true,false,null]"),       "[true,false,null]");
    expect_text(group, "array em objeto",  round_trip("{\"t\":[1,2],\"u\":\"x\"}"), "{\"t\":[1,2],\"u\":\"x\"}");
}

/* Um array aninhado dentro de outro array nao era renderizado (o render so tratava
 * objeto ou escalar como item). */
static void test_array_aninhado(void)
{
    expect_text("aninhado", "array em array", round_trip("[[1,2],[3,4]]"), "[[1,2],[3,4]]");
    expect_text("aninhado", "objetos em cadeia", round_trip("{\"a\":{\"b\":{\"c\":1}}}"), "{\"a\":{\"b\":{\"c\":1}}}");
}

/* Caso completo, no formato de um arquivo de estado que o servico relia a cada edicao:
 * strings com ':', objeto aninhado com numeros (inclusive negativo) e array vazio. */
static void test_documento_completo(void)
{
    const char* group = "documento";
    const char* doc =
        "{\"id\":\"s20260909-024657\",\"name\":\"sessao: um, dois\","
        "\"created\":\"2026-09-09T02:46:57Z\",\"state\":\"cancelled\",\"tracks\":[],"
        "\"memory\":{\"liveDelta\":-22901,\"osReservedDelta\":33554432,\"allocs\":3315,\"frees\":26216}}";

    const char* volta1 = round_trip(doc);
    const char* volta2 = volta1 ? round_trip(volta1) : 0;
    Element* t = volta2 ? parse_json(volta2) : 0;
    Element* mem;

    expect_text(group, "render estavel em 2 voltas", volta2, volta1);

    expect_text(group, "id",      field_value(t, "id"),      "s20260909-024657");
    expect_text(group, "name",    field_value(t, "name"),    "sessao: um, dois");
    expect_text(group, "created", field_value(t, "created"), "2026-09-09T02:46:57Z");
    expect_text(group, "state",   field_value(t, "state"),   "cancelled");

    mem = t ? yason_find_element(t, "memory") : 0;
    report(mem != 0, group, "objeto aninhado presente", mem ? "sim" : "ausente", "sim");
    if (mem)
    {
        expect_text(group, "numero negativo", field_value(mem, "liveDelta"), "-22901");
        expect_text(group, "numero grande",   field_value(mem, "osReservedDelta"), "33554432");
        expect_text(group, "campo do meio",   field_value(mem, "allocs"), "3315");
        expect_text(group, "ULTIMO do bloco", field_value(mem, "frees"),  "26216");
    }
}

/* ---- entrada -------------------------------------------------------------- */

int json_tests_run(void)
{
    g_pass = 0;
    g_fail = 0;

    printf("Testes de regressao JSON\n");
    printf("------------------------\n");

    test_delimitadores_dentro_de_string();
    test_valor_nao_string_no_fim_do_objeto();
    test_containers_vazios();
    test_array_na_raiz();
    test_escapes();
    test_itens_fantasma_em_array();
    test_array_aninhado();
    test_documento_completo();

    printf("------------------------\n");
    printf("%s: %d passaram, %d falharam\n\n", g_fail ? "FALHOU" : "OK", g_pass, g_fail);

    return g_fail;
}
