#include "strings.h"
#include <stdlib.h>

// TODO: make so error are propagated to 'errno'

// *** Forward Declaration *** //

// only include this 'memcpy' from c stdlib instead of pulling all <string.h>
void *memcpy(void *dest, const void *src, size_t n);
static int c_standard_string_length(const char *string, int max_size);

// *** Main funcs *** //
// default alloc strategy using malloc
static int string_init(String *str) {
  if (str == NULL)
    return -1;

  int capacity = 10;
  char *data = malloc(capacity * sizeof(char));

  if (data == NULL)
    return -2;

  str->len = 0;
  str->data = data;
  str->cap = capacity;

  return 0;
}

static void string_free(String *str) {
  if (str == NULL)
    return;

  str->len = 0;
  str->cap = 0;
  free(str->data);
}

static int string_append(String *str, const char *src) {
  if (str == NULL)
    return -1;

  int len_source = c_standard_string_length(src, 10000);
  if (len_source == 0)
    return 0;

  int needed_capacity = str->len + len_source;
  if (needed_capacity >= str->cap) {
    int new_cap = needed_capacity + 2 * str->cap;
    char *new_ptr = realloc(str->data, new_cap * sizeof(str->data));

    if (new_ptr == NULL)
      return -2;

    str->data = new_ptr;
    str->cap = new_cap;
  }

  int length = str->len;
  memcpy(&str->data[length], src, len_source);

  str->len = length + len_source;
  str->data[str->len] = '\0';

  return len_source;
}

static int c_standard_string_length(const char *string, int max_size) {
  if (string == NULL)
    return 0;

  int counter = 0;
  while (counter < max_size) {
    if (string[counter] == '\0')
      break;

    counter++;
  }

  return counter;
}

//
// *** Constant definition *** //
//

const StringNamespace strings = {
    .init = string_init,
    .free = string_free,
    .append = string_append,
};
