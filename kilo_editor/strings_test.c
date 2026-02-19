
#include "strings.h"
#include <stdio.h> // TODO: to delete after all tests are completed
#include <string.h>

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

static int my_strlen(const char *str) {
  return c_standard_string_length(str, 10000);
}

int main() {
  char *str[] = {"hell",        "hel\n",    "",         "\0",     "steve djumo",
                 "BERTold~![j", "\x1b\xFF", "\x1bi[2J", "\x1bi[H"};

  for (int i = 0; i < 9; i++) {
    int my_len = my_strlen(str[i]);
    int c_len = strlen(str[i]);

    printf("word = '%s' ::: my_len = %d ::: c_len = %d\n", str[i], my_len,
           c_len);
  }

  printf("\n\n===================\n\n");
  typedef struct {
    int x;
    const int y;
  } Person;

  Person p = {.x = 10, .y = 100};
  p.x += 33;
  // p.y += 22;

  printf("p.x = %d :: p.y = %d\n", p.x, p.y);

  printf("\n\n===================\n\n");

  String string;
  printf("init = %p\n", strings.init);
  printf("init = %d\n", strings.init(&string));

  // restart
  int succes = strings.init(&string);
  if (succes < 0)
    printf("error while initializing string");

  strings.append(&string, "je suis le best");
  strings.append(&string, ". but I have fault.");

  printf("--> str = %s\n--> len = %d\n--> cap = %d\n--> strlen = %d\n",
         string.data, string.len, string.cap, (int)strlen(string.data));

  strings.free(&string);

  return 0;
}
