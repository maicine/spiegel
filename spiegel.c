#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spiegel.h"

Vector *tokens;

int pos;
 
int consume(int ty) {
  Token *t = tokens->data[pos];

  if (t->ty != ty)
    return 0;
  pos++;
  return 1;
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
  Token *t = tokens->data[pos];

  if (consume('(')) {
    Node *node = add();
    
    if (!consume(')'))
      fprintf(stderr, "開き括弧に対応する閉じ括弧がありません: %s", t->input);
    return node;
  }

  if (t->ty == TK_NUM)
    t = tokens->data[pos++];
    return new_node_num(t->val);

  fprintf(stderr, "数値でも開き括弧でもないトークンです： %s", t->input);
}

Token *add_token(Vector *tokens, int ty, char *input) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->input = input;
  vec_push(tokens, token);
  return token;
}

void tokenize(char *p) {
  tokens = new_vector();
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      add_token(tokens, *p, p);
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      Token *t = add_token(tokens, TK_NUM, p);
      t->val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "トークナイズできません： %s\n", p);
    exit(1);
  }

  add_token(tokens, TK_EOF, p);
}

void error(int i) {
  Token *t = tokens->data[pos];
  fprintf(stderr, "予期しないトークンです： %s\n", t->input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (strcmp(argv[1],"-test") == 0) {
    runtest();
    return 0;
  }

  tokenize(argv[1]);
  Node *node = add();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

