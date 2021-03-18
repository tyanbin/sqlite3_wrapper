#include "sqlite_wrapper.h"

int main() {
  // Open a database file
  sqlite3* db = db_init("example.db");

  // Create a table
  db_exec_sql(db, "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value INTEGER, data TEXT)");

  // Use db_content to
  db_content content = content_new();
  content_insert_int(&content, "value", 1);
  content_insert_text(&content, "data", "test");

  // Insert table using content
  db_insert(db, "test", content);

  // Free content
  content_delete(content);

  // Insert table using sql
  db_exec_sql(db, "INSERT INTO test VALUES (NULL, 2, \"test\")");

  // Query from table
  db_cursor cursor = db_query(db, "test", NULL, NULL, NULL, NULL, NULL, NULL);
  if (cursor) {
    do {
      for (int i = 0; i < cursor_column_count(cursor); i++) {
        // Get the type of retrieve data
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

