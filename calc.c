#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  int_num,
  plus,
  minus,
  mul,
  div_tok,
  open_paren,
  close_paren,
  eof
} token_type;

typedef struct {
  int type;
  int value;
} token;

typedef enum {
    _NUMBER,
    _ADD,
    _SUB,
    _MUL,
    _DIV
} NodeTypes;

typedef struct Node Node;

typedef struct {
    Node *lhs;
    Node *rhs;
} NodeBinop;

typedef union {
    int number;
    NodeBinop binop;
} NodeAs;

struct Node {
    NodeTypes kind;
    NodeAs as;
};

static Node *node_num(int value) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (n == NULL) return NULL;
    n->kind = _NUMBER;
    n->as.number = value;
    return n;
}

static Node *node_binop(NodeTypes kind, Node *lhs, Node *rhs) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (n == NULL) return NULL;
    n->kind = kind;
    n->as.binop.lhs = lhs;
    n->as.binop.rhs = rhs;
    return n;
}

static void free_node(Node *n) {
    if (n == NULL) return;
    if (n->kind != _NUMBER) {
        free_node(n->as.binop.lhs);
        free_node(n->as.binop.rhs);
    }
    free(n);
}

typedef struct {
  unsigned int pos;
  const char *ip_bf;
  char c_char;
  token c_tok;
} st_t;

static int g_error;

void cerror(st_t *state, char *msg) {
  (void)state;
  if (!g_error) {
    g_error = 1;
    fprintf(stderr, "%s\n", msg);
  }
}

void consume(st_t *state) {
  if (g_error) {
    return;
  }

  state->pos++;
  state->c_char = state->ip_bf[state->pos];
}

token tk_consume(st_t *state, unsigned int type) {
  token t;

  t.type = type;
  t.value = 0;
  consume(state);
  return t;
}

int integer(st_t *state) {
  int digits;
  int result;

  digits = 0;
  result = 0;

  while (state->c_char != 0 && isdigit((unsigned char)state->c_char)) {
    if (digits >= 13) {
      cerror(state, "Number out of scope");
      return 0;
    }

    result = result * 10 + (state->c_char - '0');
    consume(state);
    digits++;
  }

  return result;
}

token tokenize(st_t *state) {
  token t;

  if (g_error) {
    t.type = eof;
    t.value = 0;
    return t;
  }

  while (state->c_char != 0) {
    char msg[64];

    if (isspace((unsigned char)state->c_char)) {
      while (state->c_char != 0 && isspace((unsigned char)state->c_char)) {
        consume(state);
      }
      continue;
    }

    if (isdigit((unsigned char)state->c_char)) {
      t.type = int_num;
      t.value = integer(state);
      return t;
    }

    if (state->c_char == '+') return tk_consume(state, plus);
    if (state->c_char == '-') return tk_consume(state, minus);
    if (state->c_char == '*') return tk_consume(state, mul);
    if (state->c_char == '/') return tk_consume(state, div_tok);
    if (state->c_char == '(') return tk_consume(state, open_paren);
    if (state->c_char == ')') return tk_consume(state, close_paren);

    sprintf(msg, "operator not found: %c", state->c_char);
    cerror(state, msg);
    t.type = eof;
    t.value = 0;
    return t;
  }

  t.type = eof;
  t.value = 0;
  return t;
}

int precedence(int type) {
  if (type == mul || type == div_tok) return 2;
  if (type == plus || type == minus) return 1;
  return 0;
}

Node *parse_expr(st_t *state, int min_prec) {
  token tok;
  Node *lhs;
  int op;
  int prec;
  NodeTypes bin_op;
  Node *rhs;

  if (g_error) return NULL;
  tok = state->c_tok;

  if (tok.type == int_num) {
    state->c_tok = tokenize(state);
    lhs = node_num(tok.value);
  } else if (tok.type == plus) {
    state->c_tok = tokenize(state);
    lhs = parse_expr(state, 3);
  } else if (tok.type == minus) {
    Node *right;
    state->c_tok = tokenize(state);
    right = parse_expr(state, 3);
    if (right == NULL) return NULL;
    lhs = node_binop(_SUB, node_num(0), right);
  } else if (tok.type == open_paren) {
    state->c_tok = tokenize(state);
    lhs = parse_expr(state, 0);
    if (state->c_tok.type != close_paren) {
      cerror(state, "Expected closing ')'");
      free_node(lhs);
      return NULL;
    }
    state->c_tok = tokenize(state);
  } else {
    cerror(state, "Expected number or '('");
    return NULL;
  }

  if (g_error || lhs == NULL) { free_node(lhs); return NULL; }

  while (!g_error) {
    op   = state->c_tok.type;
    prec = precedence(op);
    if (prec == 0 || prec < min_prec) break;

    state->c_tok = tokenize(state);
    rhs = parse_expr(state, prec + 1);
    if (rhs == NULL) { free_node(lhs); return NULL; }

    if (op == plus)        bin_op = _ADD;
    else if (op == minus)  bin_op = _SUB;
    else if (op == mul)    bin_op = _MUL;
    else                   bin_op = _DIV;

    lhs = node_binop(bin_op, lhs, rhs);
  }

  return lhs;
}

int interpret(st_t *state, Node *expr, int *out_value) {
  int lhs, rhs;

  if (g_error || expr == NULL) return 1;

  if (expr->kind == _NUMBER) {
    *out_value = expr->as.number;
    return 0;
  }

  if (interpret(state, expr->as.binop.lhs, &lhs) != 0) return 1;
  if (interpret(state, expr->as.binop.rhs, &rhs) != 0) return 1;

  if (expr->kind == _ADD) {
    *out_value = lhs + rhs;
  } else if (expr->kind == _SUB) {
    *out_value = lhs - rhs;
  } else if (expr->kind == _MUL) {
    *out_value = lhs * rhs;
  } else if (expr->kind == _DIV) {
    if (rhs == 0) {
      cerror(state, "Division by zero eror <!>");
      return 1;
    }
    *out_value = lhs / rhs;
  }

  return 0;
}

int eval(const char *input, int *out_result) {
  st_t state;
  Node *expr;
  int value;

  state.pos    = 0;
  state.ip_bf  = input;
  state.c_char = input[0];
  g_error      = 0;

  state.c_tok = tokenize(&state);
  expr        = parse_expr(&state, 0);

  if (!g_error && state.c_tok.type != eof)
    cerror(&state, "Unexpected input");

  value = 0;
  if (!g_error && interpret(&state, expr, &value) != 0) {
    free_node(expr);
    return 1;
  }

  free_node(expr);
  *out_result = value;
  return g_error;
}

