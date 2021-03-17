#include "query_builder.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t DEFAULT_STRING_SIZE = 15;

struct string_t {
  char *data;
  size_t size;
  size_t capacity;
};

static bool is_empty(const char *data) {
  return data == NULL || strlen(data) == 0;
}

static void append_clause(string str, const char *name,
                          const char *clause) {
  if (!is_empty(clause)) {
    string_append(str, name);
    string_append(str, clause);
  }
}

static char *reserve(size_t n) {
  char *data = NULL;
  data = (char *) malloc(n + 1);
  memset(data, 0, n + 1);
  return data;
}

static char *copy_of(string str, size_t size) {
  char *data = reserve(size);
  memcpy(data, str->data, str->size);
  free(str->data);
  return data;
}

static void expand_capacity(string str, size_t capacity) {
  size_t new_capacity = str->capacity * 2 + 1;
  if (new_capacity < capacity) {
    new_capacity = capacity;
  }

  printf("string capacity expand to %zu\n", new_capacity);
  str->capacity = new_capacity;
  str->data = copy_of(str, new_capacity);
}

static string string_constructor(size_t n) {
  string str = (string) malloc(sizeof(struct string_t));
  if (n > 0) {
    str->data = reserve(n);
    str->capacity = n;
    str->size = 0;
    return str;
  }

  return NULL;
}

string string_printf(const char *fmt, ...) {
  char *buffer = NULL;
  int size = 512;
  if ((buffer = calloc(1, size)) == NULL) {
    printf("malloc failed");
    return NULL;
  }

  va_list vl;
  va_start(vl, fmt);
  int nsize = vsnprintf(buffer, size, fmt, vl);
  va_end(vl);

  if (size > nsize) {
    string str = string_new_with_size(nsize);
    string_append(str, buffer);
    free(buffer);
    return str;
  } else {
    free(buffer);
    char *big_buffer = NULL;
    if ((big_buffer = calloc(1, nsize + 1)) == NULL) {
      printf("malloc failed");
      return NULL;
    }

    va_start(vl, fmt);
    vsnprintf(big_buffer, nsize + 1, fmt, vl);
    va_end(vl);

    string str = string_new_with_size(nsize);
    string_append(str, big_buffer);
    free(big_buffer);
    return str;
  }

  return NULL;
}

void string_append(string str, const char *data) {
  const size_t data_len = strlen(data);
  if (data_len == 0)
    return;

  const size_t new_len = data_len + str->size;
  if (new_len > str->capacity) {
    expand_capacity(str, new_len);
  }

  memcpy(str->data + str->size, data, data_len);
  str->size += data_len;
}

string string_new_with_size(size_t size) {
  return string_constructor(size);
}

string string_new() {
  return string_constructor(DEFAULT_STRING_SIZE);
}

const char *string_get_data(string str) {
  return str->data;
}

void string_delete(string str) {
  if (!str) {
    return;
  }

  if (str->data != NULL) {
    free(str->data);
    str->data = NULL;
  }

  free(str);
  str = NULL;
}

string build_query_string(bool distinct, const char *table, db_column columns,
                          const char *where, const char *group_by,
                          const char *having, const char *order_by,
                          const char *limit) {
  if (is_empty(group_by) && !is_empty(having)) {
    printf("HAVING clauses are only permitted when "
           "using a groupBy clause");
    return NULL;
  }

  string sql = string_new();
  string_append(sql, "SELECT ");
  if (distinct) {
    string_append(sql, "DISTINCT ");
  }

  if (columns_size(columns)) {
    for (int i = 0; i < columns_size(columns); i++) {
      if (i > 0) {
        string_append(sql, ", ");
      }
      // TODO: check columns[i] is not null
      string_append(sql, columns_get_name(columns, i));
    }
  } else {
    string_append(sql, "*");
  }

  string_append(sql, " FROM ");
  string_append(sql, table);
  append_clause(sql, " WHERE ", where);
  append_clause(sql, " GROUP BY ", group_by);
  append_clause(sql, " HAVING ", having);
  append_clause(sql, " ORDER BY ", order_by);
  append_clause(sql, " LIMIT ", limit);
  return sql;
}

string buildUpdate(const char *table, db_content data,
                   const char *where) {
  if (!data) {
    printf("Empty data");
    return NULL;
  }

  string sql = string_new();
  string_append(sql, "UPDATE ");
  string_append(sql, table);
  string_append(sql, " SET ");

  db_content cur;
  int idx = 0;
  for (cur = data; cur != NULL; cur = content_next(cur), idx++) {
    const char *key = content_get_key(cur);
    if (idx > 0) {
      string_append(sql, ", ");
    }
    string_append(sql, key);
    string_append(sql, " = ?");
  }

  append_clause(sql, " WHERE ", where);
  return sql;
}

string buildInsert(const char *table, db_content content) {
  size_t size = (content && content_size(content)) ? content_size(content) : 0;
  if (size == 0) {
    printf("Insert column is empty\n");
    return NULL;
  }

  string sql = string_new();
  string_append(sql, "INSERT INTO ");
  string_append(sql, table);
  string_append(sql, "(");

  db_content cur;
  int idx = 0;
  for (cur = content; cur != NULL; cur = content_next(cur), idx++) {
    const char *key = content_get_key(cur);
    if (idx > 0) {
      string_append(sql, ", ");
    }

    string_append(sql, key);
  }

  string_append(sql, ") VALUES (");
  for (int i = 0; i < idx; i++) {
    if (i > 0) {
      string_append(sql, ", ");
    }

    string_append(sql, "?");
  }

  string_append(sql, ")");
  return sql;
}