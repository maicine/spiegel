enum {
  TK_NUM = 256, // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
};

typedef struct {
  int ty;  // トークンの型が入る
  int val; // ty がTK_NUMの場合、その数値
  char *input;
} Token;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

enum {
  ND_NUM = 256, // 整数のノードの型
};

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node *lhs; // 左辺　left hand side
  struct node *rhs; // 右辺　right hand side
  int val;          // tyがND_NUMの場合のみ使う
} Node;

int consume(int ty);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *add();
Node *mul();
Node *term();
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Token *add_token(Vector *v, int ty, char *input);
void tokenize(char *p);
void gen(Node *node);
void error(int i);

/* util_test.c */
int expect(int line, int expected, int actual);
void runtest();