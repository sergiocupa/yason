#include "yason_compat.h"
#include <ctype.h>
#include <stdio.h>

// fopen_s e da CRT do MSVC. Fora do Windows este arquivo nao compilava (a yason nao
// linkava no Linux): mesmo contrato -- 0 em sucesso, errno em falha.
#ifndef _WIN32
#include <errno.h>
static int fopen_s(FILE** f, const char* path, const char* mode)
{
    *f = fopen(path, mode);
    return *f ? 0 : errno;
}
#endif

static int yason_cstr_length(const char* data) {
    int n = 0;
    if (data) while (data[n]) ++n;
    return n;
}

static int yason_bytes_equal(const char* a, const char* b, int length) {
    int i;
    for (i = 0; i < length; ++i) if (a[i] != b[i]) return 0;
    return 1;
}

StringX* yason_string_new(void) {
    StringX* str = (StringX*)memop_alloc_raw(sizeof(StringX));
    if (str) string_init(str);
    return str;
}

void yason_string_append_sub(StringX* dst, const char* data, int data_length, int start, int count) {
    if (!dst || !data || start < 0 || start > data_length || count < 0) return;
    if (count > data_length - start) count = data_length - start;
    string_appends(dst, data, data_length, start, count);
}

void yason_string_append(StringX* dst, const char* data) {
    if (data) yason_string_append_sub(dst, data, yason_cstr_length(data), 0, yason_cstr_length(data));
}

void yason_string_append_char(StringX* dst, char data) {
    yason_string_append_sub(dst, &data, 1, 0, 1);
}

void yason_string_init_sub(StringX* dst, const char* data, int data_length, int start, int count) {
    string_init(dst);
    yason_string_append_sub(dst, data, data_length, start, count);
}

int yason_string_with_content(const StringX* str) { return str && str->Length > 0; }

int yason_string_equals(const StringX* str, const char* data) {
    int n = yason_cstr_length(data);
    return str && data && str->Length == (uint64)n && yason_bytes_equal(str->Content, data, n);
}

int yason_string_equals_char(const char* a, const char* b) { int n = yason_cstr_length(a); return a && b && n == yason_cstr_length(b) && yason_bytes_equal(a, b, n); }

int yason_string_equals_char_range(const char* a, const char* b, int start, int count) {
    int alen, blen;
    if (!a || !b || start < 0) return 0;
    alen = yason_cstr_length(a); blen = yason_cstr_length(b);
    if (start > alen) return 0;
    if (count < 0) count = alen - start;
    return count == blen && yason_bytes_equal(a + start, b, blen);
}

char* yason_string_to_upper_copy(const char* data) {
    int i, n;
    char* out;
    if (!data) return NULL;
    n = yason_cstr_length(data); out = (char*)memop_alloc_raw(n + 1);
    if (!out) return NULL;
    for (i = 0; i < n; ++i) out[i] = (data[i] >= 'a' && data[i] <= 'z') ? (char)(data[i] - ('a' - 'A')) : data[i];
    out[n] = 0; return out;
}

int yason_string_index_end_char(const char* data, char token) {
    int i = yason_cstr_length(data);
    while (i-- > 0) if (data[i] == token) return i;
    return -1;
}

static int token_match(char c, const char* tokens, int token_length) {
    int i; for (i = 0; i < token_length; ++i) if (c == tokens[i]) return i; return -1;
}

int yason_string_index_first(const char* data, int data_length, const char* tokens, int token_length, int start, int* position) {
    int i, match;
    if (position) *position = -1;
    if (!data || !tokens || start < 0) return -1;
    for (i = start; i < data_length; ++i) if ((match = token_match(data[i], tokens, token_length)) >= 0) { if (position) *position = i; return match; }
    return -1;
}

int yason_string_token_count(const char* data, int data_length, char token, int start, int count, int* position) {
    int end = start + count, i = start;
    if (end > data_length) end = data_length;
    while (i < end && data[i] == token) ++i;
    if (position) *position = i;
    return i - start;
}

YasonStringArray* yason_string_split_first_char(const char* data, int length, const char* tokens, int token_length) {
    int start = 0, i;
    YasonStringArray* ar = (YasonStringArray*)memop_alloc_raw(sizeof(YasonStringArray));
    if (!ar) return NULL;
    ar->Max = 16; ar->Count = 0; ar->Items = (StringX**)memop_alloc_raw(sizeof(StringX*) * ar->Max);
    for (i = 0; i <= length; ++i) {
        if (i == length || token_match(data[i], tokens, token_length) >= 0) {
            StringX* part = yason_string_new(); yason_string_append_sub(part, data, length, start, i - start);
            if (ar->Count == ar->Max) { ar->Max *= 2; ar->Items = (StringX**)memop_realloc_raw(ar->Items, sizeof(StringX*) * ar->Max); }
            ar->Items[ar->Count++] = part; start = i + 1;
        }
    }
    return ar;
}

void yason_string_trim_end_by_first_char(StringX* str, const char* tokens) {
    if (!str || !tokens) return;
    while (str->Length && token_match(str->Content[str->Length - 1], tokens, yason_cstr_length(tokens)) >= 0) --str->Length;
    str->Content[str->Length] = 0;
}

ListX* yason_list_create(uint64 type_size) { return list_create(INITIAL_LIST_COUNT, type_size); }

void yason_list_release(ListX* value) {
    ListX* list = value;
    if (!list) return;
    list_release(&list);
    memop_free_raw(value);
}

int file_write_text(const char* path, char* content, int length) {
    FILE* f; size_t written;
    if (fopen_s(&f, path, "wb") != 0) return 0;
    written = fwrite(content, 1, (size_t)length, f); fclose(f); return written == (size_t)length;
}

int file_read_text(const char* path, char** out, int* out_length) {
    FILE* f; long length; char* data;
    if (!out || !out_length || fopen_s(&f, path, "rb") != 0) return 0;
    fseek(f, 0, SEEK_END); length = ftell(f); rewind(f);
    if (length < 0) { fclose(f); return 0; }
    data = (char*)memop_alloc_raw((uint64)length + 1);
    if (!data) { fclose(f); return 0; }
    if (fread(data, 1, (size_t)length, f) != (size_t)length) { fclose(f); memop_free_raw(data); return 0; }
    fclose(f); data[length] = 0; *out = data; *out_length = (int)length; return 1;
}
