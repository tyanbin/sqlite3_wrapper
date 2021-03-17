#include "cursor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cursor_t {
  // sqlite3* db is used to print errmsg
  sqlite3 *db;
  sqlite3_stmt *stmt;
};

db_cursor cursor_new(sqlite3 *db, sqlite3_stmt *stmt) {
  db_cursor cursor = (db_cursor) malloc(sizeof(struct cursor_t));
  cursor->stmt = stmt;
  cursor->db = db;
  return cursor;
}

db_cursor cursor_next(db_cursor cursor) {
  int rc = sqlite3_step(cursor->stmt);
  if (rc == SQLITE_ROW) {
    return cursor;
  } else if (rc != SQLITE_DONE) {
    printf("get next cursor error: %s\n", sqlite3_errmsg(cursor->db));
    return NULL;
  }

  printf("process all rows done\n");
  return NULL;
}

int cursor_get_int(db_cursor cursor, int col) {
  return sqlite3_column_int(cursor->stmt, col);
}

const char *cursor_get_text(db_cursor cursor, int col) {
  return (const char *) sqlite3_column_text(cursor->stmt, col);
}

double cursor_get_double(db_cursor cursor, int col) {
  return sqlite3_column_double(cursor->stmt, col);
}

value_type cursor_get_column_type(db_cursor cursor, int col) {
  value_type type = VALUE_NULL;
  int t = sqlite3_column_type(cursor->stmt, col);
  switch (t) {
    case SQLITE_INTEGER:type = VALUE_INT;
      break;
    case SQLITE_TEXT:type = VALUE_TEXT;
      break;
    case SQLITE_FLOAT:type = VALUE_DOUBLE;
      break;
    case SQLITE_BLOB:type = VALUE_BLOB;
      break;
    case SQLITE_NULL:type = VALUE_NULL;
      break;
    default:printf("not support type: %d\n", t);
  }

  return type;
}

const char *cursor_get_column_name(db_cursor cursor, int col) {
  return sqlite3_column_name(cursor->stmt, col);
}

int cursor_column_count(db_cursor cursor) {
  return sqlite3_column_count(cursor->stmt);
}

int cursor_get_column_index(db_cursor cursor, const char *column_name) {
  printf("cursor_get_column_index: %d\n", cursor_column_count(cursor));
  for (int i = 0; i < cursor_column_count(cursor); i++) {
    const char *name = cursor_get_column_name(cursor, i);
    printf("cursor_get_column_index: %s, %s\n", name, column_name);
    if (strcmp(name, column_name) == 0) {
      return i;
    }
  }

  return -1;
}

void cursorDelete(db_cursor cursor) {
  if (!cursor)
    return;

  sqlite3_finalize(cursor->stmt);
  free(cursor);
}