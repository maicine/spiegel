#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256, // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
};

typedef struct {
  int ty;  // トークンの型が入る
  int val; // ty がTK_NUMの場合、その数値
  char *input;
} Token;

enum {
  ND_NUM = 256, // 整数のノードの型
};

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node *lhs; // 左辺　left hand side
  struct node *rhs; // 右辺　right hand side
  int val;          // tyがND_NUMの場合のみ使う
} Node;

Token tokens[100];
int pos;

/* proto type */
int consume(int ty);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *add();
Node *mul();
Node *term();
void tokenize(char *p);
void gen(Node *node);
void error(int i);

int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      fprintf(stderr, "開き括弧に対応する閉じ括弧がありません: %s", tokens[pos].input);
    return node;
  }

  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  fprintf(stderr, "数値でも開き括弧でもないトークンです： %s", tokens[pos].input);
}

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "トークナイズできません： %s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty){
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf(" mov rdx, 0\n");
    printf(" div rdi\n");
  }
  
  printf("  push rax\n");
}

void error(int i) {
  fprintf(stderr, "予期しないトークンです： %s\n", tokens[i].input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  tokenize(argv[1]);
  Node *node = add();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  // if (tokens[0].ty != TK_NUM)
  //   error(0);
  // printf("  mov rax, %d\n", tokens[0].val);

  // int i = 1;
  // while (tokens[i].ty != TK_EOF) {
  //   if (tokens[i].ty == '+') {
  //     i++;
  //     if (tokens[i].ty != TK_NUM)
  //       error(i);
  //     printf("  add rax, %d\n", tokens[i].val);
  //     i++;
  //     continue;
  //   }

  //   if (tokens[i].ty == '-') {
  //     i++;
  //     if (tokens[i].ty != TK_NUM)
  //       error(i);
  //     printf("  sub rax, %d\n", tokens[i].val);
  //     i++;
  //     continue;
  //   }

  //   error(i);
  // }

  printf("  pop rax\n");

  printf("  ret\n");
  return 0;
}