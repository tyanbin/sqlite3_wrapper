
#ifndef CONTENT_H
#define CONTENT_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum value_type {
    VALUE_NULL = 1,    /* Value is NULL (or a pointer) */
    VALUE_TEXT,        /* Value is a string */
    VALUE_INT,         /* Value is an integer */
    VALUE_DOUBLE,       /* Value is a float number */
    VALUE_BLOB         /* Value is a blob */
} value_type;

typedef struct content_t* db_content;
typedef struct value_t* db_value;
typedef struct column_t* db_column;

db_content content_new();
void content_delete(db_content data);
void content_insert_text(db_content* data, const char* key, const char* s);

void content_insert_int(db_content* data, const char* key, int i);
void content_insert_double(db_content* data, const char* key, double d);

void content_erase(db_content* data, const char* key);

const char* content_get_text(db_value value);
int content_get_int(db_value value);
double content_get_double(db_value data);

db_value content_get_value(db_content data, const char* key);
value_type content_get_type(db_value value);

const char* content_get_text_by_key(db_content data, const char* key);
int content_get_int_by_Key(db_content data, const char* key);
double content_get_double_by_key(db_content data, const char* key);

db_content content_next(db_content current);
const char* content_get_key(db_content current);
size_t content_size(db_content data);
bool content_has_key(db_content data, const char* key);

// column set
db_column columns_new();
db_column columns_new_with_name(int column_num, ...);
void columns_push(db_column* columns, const char* column);
const char* columns_get_name(db_column columns, size_t index);
size_t columns_size(db_column columns);
void columns_delete(db_column columns);

#ifdef __cplusplus
}
#endif

#endif  // CONTENT_H