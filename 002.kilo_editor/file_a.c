#include "file.h"
#include <stdio.h>
#include <stdlib.h>

struct Code {
  int lang_ide;
};

struct Code *create_code(int lang_id) {
  struct Code *code = malloc(sizeof(struct Code));
  if (code == NULL)
    return NULL;

  code->lang_ide = lang_id;
  return code;
}

void print_code(struct Code *code) {
  printf("code.lang_id = %d\n", code->lang_ide);
}

void print_position(Position pos) {
  printf("pos.x = %d :: pos.y = %d \n", pos.x, pos.y);
}
