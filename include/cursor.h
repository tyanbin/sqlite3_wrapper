#ifndef CURSOR_H
#define CURSOR_H

#include <sqlite3.h>

#include "content.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cursor_t* db_cursor;

// Create a cursor
db_cursor cursor_new(sqlite3* db, sqlite3_stmt* stmt);
db_cursor cursor_next(db_cursor cursor);
int cursor_get_int(db_cursor cursor, int col);
const char* cursor_get_text(db_cursor cursor, int col);
double cursor_get_double(db_cursor cursor, int col);
int cursor_column_count(db_cursor cursor);
const char* cursor_get_column_name(db_cursor cursor, int col);
value_type cursor_get_column_type(db_cursor cursor, int col);
int cursor_gset_column_index(db_cursor cursor, const char* column_name);

// Destroy a cursor
void cursorDelete(db_cursor cursor);

#ifdef __cplusplus
}
#endif

#endif  // DATABASE_CURSOR_H_