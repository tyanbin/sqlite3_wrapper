#ifndef SQLITE_WRAPPER_H
#define SQLITE_WRAPPER_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "content.h"
#include "cursor.h"
#include "monitor.h"

#ifdef __cplusplus
extern "C" {
#endif

sqlite3* db_init(const char* db_name);
void db_deinit(sqlite3* db);

db_cursor db_query(sqlite3* db, const char* table, db_column columns,
                   const char* where, const char* group_by,
                   const char* having, const char* order_by, const char* limit);
int db_update(sqlite3* db, const char* table, db_content content,
              const char* where);
int db_insert(sqlite3* db, const char* table, db_content content);
int db_delete(sqlite3* db, const char* table, const char* where);

int db_exec_sql(sqlite3* db, const char* sql);
db_cursor db_query_sql(sqlite3* db, const char* sql);

bool db_column_exists(sqlite3* db, const char* table, const char* column);

// Get the column names and count;
char** db_get_table_info(sqlite3* db, const char* table, int* column_count);

#ifdef __cplusplus
}
#endif

#endif  // SQLITE_WRAPPER_H