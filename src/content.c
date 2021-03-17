#include "content.h"

#include <stdarg.h>
#include <string.h>

#include "uthash.h"

static const size_t DEFAULT_COLUMNS_SIZE = 4;

struct value_t {
  double d;     /* Float value used when MEM_Real is set in flags */
  int64_t i;    /* Integer value used when MEM_Int is set in flags */
  char *s;      /* String value used when valueStr is set in flags */

  value_type flag;
};

struct content_t {
  db_value value;
  char *key;     /* String value for column name */
  UT_hash_handle hh;
};

struct column_t {
  char **names;
  size_t size;
  size_t capacity;
};

static db_content contentConstructor(const char *key) {
  db_content data =
      (db_content) calloc(1, sizeof(struct content_t));
  data->key = (char *) malloc(strlen(key) + 1);
  strcpy(data->key, key);
  data->value =
      (db_value) calloc(1, sizeof(struct value_t));
  data->value->d = 0;
  data->value->i = 0;
  data->value->s = NULL;
  data->value->flag = VALUE_NULL;
  return data;
}

db_content content_new() {
  db_content data = NULL;
  return data;
}

void content_insert_text(db_content *data, const char *key, const char *s) {
  if (!s)
    return;

  db_content tmp;
  HASH_FIND_STR(*data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL) {
    tmp = contentConstructor(key);
    HASH_ADD_STR(*data, key, tmp);
  }

  tmp->value->flag = VALUE_TEXT;
  tmp->value->s = (char *) malloc(strlen(s) + 1);
  strcpy(tmp->value->s, s);
}

void content_insert_int(db_content *data, const char *key, int i) {
  db_content tmp;
  HASH_FIND_STR(*data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL) {
    tmp = contentConstructor(key);
    HASH_ADD_STR(*data, key, tmp);
  }

  tmp->value->flag = VALUE_INT;
  tmp->value->i = i;
}

void content_insert_double(db_content *data, const char *key, double d) {
  db_content tmp;
  HASH_FIND_STR(*data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL) {
    tmp = contentConstructor(key);
    HASH_ADD_STR(*data, key, tmp);
  }

  tmp->value->flag = VALUE_DOUBLE;
  tmp->value->d = d;
}

void content_erase(db_content *data, const char *key) {
  db_content tmp;
  HASH_FIND_STR(*data, key, tmp);
  if (tmp != NULL) {
    HASH_DEL(*data, tmp);  /* user: pointer to deletee */
    if (tmp->key) {
      free(tmp->key);
    }

    if (tmp->value) {
      if (tmp->value->flag == VALUE_TEXT) {
        free(tmp->value->s);
      }

      free(tmp->value);
    }

    free(tmp);
  }
}

const char *content_get_text_by_key(db_content data,
                                    const char *key) {
  db_content tmp;
  HASH_FIND_STR(data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL || tmp->value->flag != VALUE_TEXT) {
    return NULL;
  }

  return tmp->value->s;
}

int content_get_int_by_Key(db_content data, const char *key) {
  db_content tmp;
  HASH_FIND_STR(data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL || tmp->value->flag != VALUE_INT) {
    return 0;
  }

  return tmp->value->i;
}

double content_get_double_by_key(db_content data, const char *key) {
  db_content tmp;
  HASH_FIND_STR(data, key, tmp);  /* value already in the hash? */
  if (tmp == NULL || tmp->value->flag != VALUE_DOUBLE) {
    return 0;
  }

  return tmp->value->d;
}

void content_delete(db_content data) {
  if (!data)
    return;

  db_content current, tmp;
  HASH_ITER(hh, data, current, tmp) {
    HASH_DEL(data, current);
    if (current->key) {
      free(current->key);
    }

    if (current->value) {
      if (current->value->flag == VALUE_TEXT) {
        free(current->value->s);
      }

      free(current->value);
    }

    free(current);
  }
}

db_content content_next(db_content current) {
  return current ? (db_content) current->hh.next : NULL;
}

value_type content_get_type(db_value value) {
  if (!value)
    return VALUE_NULL;

  return value->flag;
}

const char *content_get_key(db_content current) {
  if (!current)
    return NULL;

  return current->key;
}

size_t content_size(db_content data) {
  return HASH_COUNT(data);
}

bool content_has_key(db_content data, const char *key) {
  db_content tmp;
  HASH_FIND_STR(data, key, tmp);
  return tmp != NULL;
}

db_value content_get_value(db_content data,
                           const char *key) {
  db_content tmp;
  HASH_FIND_STR(data, key, tmp);
  return tmp ? tmp->value : NULL;
}

const char *content_get_text(db_value value) {
  if (value && value->flag == VALUE_TEXT) {
    return value->s;
  }

  return NULL;
}

int content_get_int(db_value value) {
  if (value && value->flag == VALUE_INT) {
    return value->i;
  }

  return 0;
}

double content_get_double(db_value data) {
  if (data && data->flag == VALUE_DOUBLE) {
    return data->d;
  }

  return 0;
}

void columns_push(db_column *columns, const char *column) {
  const size_t len = strlen(column);
  if (len == 0)
    return;

  db_column cur = *columns;
  if ((cur)->size + 1 > (cur)->capacity) {
    (cur)->capacity *= 2;
    char **tmp = (char **) calloc(1, sizeof(char *) * (cur)->capacity);
    for (int i = 0; i < cur->size; i++) {
      tmp[i] = cur->names[i];
    }

    free((cur)->names);
    cur->names = tmp;
  }

  cur->names[cur->size] = malloc(strlen(column) + 1);
  strcpy(cur->names[cur->size], column);
  cur->size++;
}

db_column columns_new() {
  db_column columns = (db_column) malloc(sizeof(struct column_t));
  columns->names = (char **) calloc(1, sizeof(char *) * DEFAULT_COLUMNS_SIZE);
  columns->size = 0;
  columns->capacity = DEFAULT_COLUMNS_SIZE;
  return columns;
}

db_column columns_new_with_name(int column_num, ...) {
  db_column columns = (db_column) malloc(sizeof(struct column_t));
  columns->names = (char **) calloc(1, sizeof(char *) * column_num);
  columns->size = 0;
  va_list vl;
  va_start(vl, column_num);
  for (int i = 0; i < column_num; i++) {
    char *column = va_arg(vl, char*);
    columns->names[columns->size] = malloc(strlen(column) + 1);
    strcpy(columns->names[columns->size++], column);
  }

  va_end(vl);
  return columns;
}

size_t columns_size(db_column columns) {
  if (!columns)
    return 0;

  return columns->size;
}

const char *columns_get_name(db_column columns, size_t index) {
  if (index > columns->size)
    return NULL;

  return columns->names[index];
}

void columns_delete(db_column columns) {
  if (!columns)
    return;

  for (int i = 0; i < columns->size; i++) {
    if (columns->names[i]) {
      free(columns->names[i]);
    }
  }

  free(columns->names);
  free(columns);
}