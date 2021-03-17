#ifndef QUERY_BUILDER_H
#define QUERY_BUILDER_H

#include <stdbool.h>
#include <stdio.h>

#include "content.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct string_t* string;

string string_new();
string string_new_with_size(size_t size);
void string_delete(string str);
void string_append(string str, const char* data);
string string_printf(const char* fmt, ...);
const char* string_get_data(string str);

string build_query_string(bool distinct, const char* table,
                          db_column columns, const char* where, const char* group_by,
                          const char* having, const char* order_by, const char* limit);

string buildUpdate(const char* table, db_content content, const char* where);
string buildInsert(const char* table, db_content content);

#ifdef __cplusplus
}
#endif

#endif  // QUERY_BUILDER_H