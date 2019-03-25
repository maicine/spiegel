#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spiegel.h"

Vector *tokens;
Node *code[100];
int pos=0;
 
int consume(int ty) {
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    return 0;
  pos++;
  return 1;
}

void program() {
  int i = 0;
  Token *t = tokens->data[pos];
  for(;;){
    t = tokens->data[pos];
    if(t->ty != TK_EOF)
      code[i++] = stmt();
    else
      return;
  }
  code[i] = NULL;
}

Node *stmt() {
  Node *node = assign();
  
  Token *t = tokens->data[pos];
  // pos++;
  // if (t->ty == ';') {
  //   t = tokens->data[pos++];
  //   return new_node_num(t->val);
  // }

  if (!consume(';'))
    fprintf(stderr, "';'ではないトークンです: %c\n", t->ty);
  return node;
}

Node *assign() {
  Node *node = add();

  for (;;) {
    if (consume('='))
      node = new_node('=', node, assign());
    else
      return node;
  }
  // if (consume(TK_EOF)) {
  //   return node;
  // }  
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

  if (t->ty == TK_NUM) {
    t = tokens->data[pos++];
    return new_node_num(t->val);
  }
  if (t->ty == TK_IDENT) {
    t = tokens->data[pos++];
    return new_node_ident(t->val);
  }

  if (consume('(')) {
    Node *node = assign();
    
    if (!consume(')'))
      fprintf(stderr, "開き括弧に対応する閉じ括弧がありません: %s\n", t->input);
    return node;
  }

  fprintf(stderr, "数値でも開き括弧でもないトークンです： %s\n", t->input);
  return NULL;
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
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      add_token(tokens, *p, p);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      add_token(tokens, TK_IDENT, p);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      Token *t = add_token(tokens, TK_NUM, p);
      t->val = strtol(p, &p, 10);
      continue;
    }

    fprintf(stderr, "トークナイズできません： %s\n", *p);
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
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 先頭の式から順次コード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

