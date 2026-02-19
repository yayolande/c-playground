
struct Code;
struct Code *create_code(int lang_id);
void print_code(struct Code *code);

// =============================================

typedef struct {
  int x, y;
} Position;

void print_position(Position pos);
