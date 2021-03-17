#include "sqlite_wrapper.h"

int main() {
  sqlite3* db = db_init("example.db");
  db_exec_sql(db, "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT)");
  db_exec_sql(db, "INSERT INTO test VALUES (NULL, \"test\")");
  db_cursor cursor = db_query(db, "test", NULL, NULL, NULL, NULL, NULL, NULL);
  if (cursor) {
    do {
      for (int i = 0; i < cursor_column_count(cursor); i++) {
        value_type type = cursor_get_column_type(cursor, i);
        switch (type) {
          case VALUE_INT:
            printf("%s, %d\n", cursor_get_column_name(cursor, i),
                   cursor_get_int(cursor, i));
            break;
          case VALUE_TEXT:
            printf("%s, %s\n", cursor_get_column_name(cursor, i),
                   cursor_get_text(cursor, i));
            break;
          default:
            break;
        }
      }
    } while (cursor_next(cursor));
  }

  return 0;
}
