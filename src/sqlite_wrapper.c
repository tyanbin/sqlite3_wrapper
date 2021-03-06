#include "sqlite_wrapper.h"

#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#include "query_builder.h"

static char *g_db_err_msg = NULL;

static int64_t get_time_in_ms() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return (int64_t)(now.tv_sec * 1000 + now.tv_usec / 1000);
}

static int try_exec(sqlite3 *db, const char *sql) {
  printf("exec sql: %s\n", sql);
  int64_t start = get_time_in_ms();
  int rc = sqlite3_exec(db, sql, NULL, NULL, &g_db_err_msg);
  if (rc != SQLITE_OK) {
    printf("SQL error: %s\n", g_db_err_msg);
    sqlite3_free(g_db_err_msg);
    g_db_err_msg = NULL;
  } else {
    printf("Operation done successfully\n");
  }

  printf("exec sql end (%lld ms)\n", get_time_in_ms() - start);
  return rc;
}

static void bind_arguments(sqlite3_stmt *sqlit_stmt,
                           db_content content) {
  db_content cur;
  int idx = 1;
  for (cur = content; cur != NULL; cur = content_next(cur), idx++) {
    const char *key = content_get_key(cur);
    db_value value = content_get_value(cur, key);
    value_type type = content_get_type(value);
    switch (type) {
      case VALUE_TEXT:
        sqlite3_bind_text(sqlit_stmt, idx, content_get_text(value),
                          -1, SQLITE_STATIC);
        break;
      case VALUE_INT: {
        sqlite3_bind_int(sqlit_stmt, idx, content_get_int(value));
        break;
      }
      case VALUE_DOUBLE: {
        sqlite3_bind_double(sqlit_stmt, idx,
                            content_get_double(value));
        break;
      }
      case VALUE_NULL: {
        sqlite3_bind_null(sqlit_stmt, idx);
        break;
      }
      default:printf("Not support type: %d\n", type);
        return;
    }
  }
}

static sqlite3_stmt *try_step(sqlite3 *db, const char *sql) {
  printf("try step sql: %s\n", sql);
  sqlite3_stmt *stmt = NULL;
  sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
  int rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
      printf("process all rows done\n");
      return NULL;
    }

    printf("failed to execute sql: %s, error(%d): %s\n",
           sql, rc, sqlite3_errmsg(db));
    return NULL;
  }

  return stmt;
}

static int try_single_step(sqlite3 *db, const char *sql,
                           db_content args) {
  sqlite3_stmt *stmt = NULL;
  sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
  printf("try single step sql: %s\n", sql);
  if (args)
    bind_arguments(stmt, args);

  int rc = sqlite3_step(stmt);
  // Suppose used for UPDATA, INSTER, so no row return.
  if (rc != SQLITE_DONE) {
    printf("insert error(%d): %s\n", rc, sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return rc != SQLITE_DONE ? rc : SQLITE_OK;
}

static void callback(void *udp, int type,
                     const char *db_name, const char *tbl_name, sqlite3_int64 rowid) {
  printf("db_name: %s, tbl_name: %s, type: %d, rowid: %lld\n",
         db_name, tbl_name, type, rowid);

  int *id = (int *) udp;
  *id = rowid;
}

static void update_callback(sqlite3 *db, void *udp) {
  sqlite3_update_hook(db, callback, udp);
}

static void disable_callback(sqlite3 *db) {
  sqlite3_update_hook(db, NULL, NULL);
}

db_cursor db_query(sqlite3 *db, const char *table,
                   db_column columns, const char *where,
                   const char *group_by, const char *having, const char *order_by,
                   const char *limit) {
  string sql = build_query_string(false, table, columns,
                                  where, group_by, having, order_by, limit);
  sqlite3_stmt *stmt = try_step(db, string_get_data(sql));
  string_delete(sql);
  return stmt ? cursor_new(db, stmt) : NULL;
}

int db_update(sqlite3 *db, const char *table, db_content content,
              const char *where) {
  printf("update start =>\n");
  int64_t start = get_time_in_ms();
  string sql = buildUpdate(table, content, where);
  int rc = try_single_step(db, string_get_data(sql), content);
  string_delete(sql);
  printf("=> update end (%lld ms)\n", get_time_in_ms() - start);
  return rc;
}

int db_insert(sqlite3 *db, const char *table, db_content content) {
  printf("insert start =>\n");
  int64_t start = get_time_in_ms();
  string sql = buildInsert(table, content);
  int rc = try_single_step(db, string_get_data(sql), content);
  string_delete(sql);
  printf("=> insert end (%lld ms)\n", get_time_in_ms() - start);
  return rc;
}

int db_delete(sqlite3 *db, const char *table, const char *where) {
  printf("delete start =>\n");
  int64_t start = get_time_in_ms();
  string sql = string_new();
  string_append(sql, "DELETE FROM ");
  string_append(sql, table);
  string_append(sql, " WHERE ");
  string_append(sql, where);
  int rc = try_exec(db, string_get_data(sql));
  string_delete(sql);
  printf("=> delete end (%lld ms)\n", get_time_in_ms() - start);
  return rc;
}

sqlite3 *db_init(const char *db_name) {
  sqlite3 *db = NULL;
  sqlite3_open(db_name, &db);
//    sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_CONFIG_MULTITHREAD, 0);
  if (!db) {
    printf("failed to open DB\n");
    return NULL;
  }

  return db;
}

void db_deinit(sqlite3 *db) {
  sqlite3_close(db);
}

int db_exec_sql(sqlite3 *db, const char *sql) {
  return try_exec(db, sql);
}

db_cursor db_query_sql(sqlite3 *db, const char *sql) {
  sqlite3_stmt *stmt = try_step(db, sql);
  return stmt ? cursor_new(db, stmt) : NULL;
}

bool db_column_exists(sqlite3 *db, const char *table, const char *column) {
  string sql = string_printf("PRAGMA table_info(%s)", table);
  char **result = NULL;
  int row = 0, col = 0;
  char *err = NULL;
  sqlite3_get_table(db, string_get_data(sql), &result, &row, &col, &err);
  int idx = col;
  bool exist = false;
  for (int i = 0; i < row; i++) {
    if (strcmp(result[idx + 1], column) == 0) {
      exist = true;
      break;
    }

    idx += col;
  }

  sqlite3_free_table(result);
  string_delete(sql);
  return exist;
}

char **db_get_table_info(sqlite3 *db, const char *table,
                         int *column_count) {
  string sql = string_new();
  string_append(sql, "PRAGMA table_info(");
  string_append(sql, table);
  string_append(sql, ")");
  char **result = NULL;
  int row = 0, column = 0;
  char *errmsg = NULL;
  sqlite3_get_table(db, string_get_data(sql), &result, &row, &column,
                    &errmsg);

  char **columns = (char **) malloc(sizeof(char *) * row);
  *column_count = row;
  int idx = column;
  for (int i = 0; i < row; i++) {
    const char *column_name = result[idx + 1];
    columns[i] = malloc(strlen(column_name) + 1);
    strcpy(columns[i], column_name);
    idx += column;
  }

  sqlite3_free_table(result);
  string_delete(sql);

  return columns;
}