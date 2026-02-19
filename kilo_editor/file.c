#include "file.h"
#include <stdio.h>

int main() {
  Position pos = {.x = 10, .y = 43};
  struct Code *go_code = create_code(10);

  print_position(pos);

  printf("\n\n============\n\n");
  printf("Code = %p\n", go_code);
  print_code(go_code);

  return 0;
}
