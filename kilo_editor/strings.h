
typedef struct {
  char *data;
  int len;
  int cap;
} String;

// Used as a namespace for all available static functions
typedef struct {
  int (*init)(String *str);
  void (*free)(String *str);
  int (*append)(String *str, const char *src);
} StringNamespace;

extern const StringNamespace strings;
