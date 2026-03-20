#include <stdlib.h>
#include <string.h>
#include "parse.h"

typedef Node *(*prefix_func)(CalcCtx *, Token **, Token *);
typedef Node *(*infix_func)(CalcCtx *, Token **, Token *, Node *);

typedef struct {
  int         l_bp;
  int         r_bp;
  prefix_func prefix;
  infix_func  infix;
} ParseRule;


static Node *parse_expr(CalcCtx *ctx, Token **rest, Token *tok, int min_bp);


static ParseRule rules[TK_COUNT];

/* helper functions*/

static void consume(Token **rest) {
  *rest = (*rest)->next;
}

static OpType tok_to_op(Tokentype t) {
  switch (t) {
  case TK_PLUS:  return OP_ADD;
  case TK_MINUS: return OP_SUB;
  case TK_MUL:   return OP_MUL;
  case TK_DIV:   return OP_DIV;
  case TK_POW:   return OP_POW;
  default:       return OP_ADD;
  }
}

/* Node constructor functions   */

static Node *new_node(CalcCtx *ctx, NodeKind kind, Token *tok) {
  Node *n = calloc(1, sizeof(Node));
  if (n != NULL) {
    n->kind = kind;
    n->tok  = tok;
    return n;
  }
  ctx_error(ctx, "out of memory", NULL, 0);
  return NULL;
}

static Node *new_binary(CalcCtx *ctx, OpType op, Node *lhs, Node *rhs, Token *tok) {
  Node *n = new_node(ctx, NODE_BINARY, tok);
  n->as.binop.op  = op;
  n->as.binop.lhs = lhs;
  n->as.binop.rhs = rhs;
  return n;
}

static Node *new_literal_int(CalcCtx *ctx, long val, Token *tok) {
  Node *n = new_node(ctx, NODE_LITERAL, tok);
  n->as.literal.type    = VAL_INT;
  n->as.literal.as.ival = val;
  return n;
}

static Node *new_literal_float(CalcCtx *ctx, double val, Token *tok) {
  Node *n = new_node(ctx, NODE_LITERAL, tok);
  n->as.literal.type    = VAL_FLOAT;
  n->as.literal.as.fval = val;
  return n;
}

/* Pratt parser prefix handling , for single operand and unary operations*/

static Node *prefix_int(CalcCtx *ctx, Token **rest, Token *tok) {
  consume(rest);
  return new_literal_int(ctx, strtol(tok->loc, NULL, 10), tok);
}

static Node *prefix_float(CalcCtx *ctx, Token **rest, Token *tok) {
  consume(rest);
  return new_literal_float(ctx, strtod(tok->loc, NULL), tok);
}

static Node *prefix_ident(CalcCtx *ctx, Token **rest, Token *tok) {
  Token *next = tok->next;
  Node  *arg;
  Node  *n;

  if (next != NULL && next->type == TK_LPAREN) {
    *rest = next->next;
    arg   = parse_expr(ctx, rest, *rest, 0);
    if (*rest == NULL || (*rest)->type != TK_RPAREN)
      ctx_error(ctx, "expected ')' after function argument", NULL, 0);
    consume(rest);
    n = new_node(ctx, NODE_CALL, tok);
    n->as.call.name = tok->loc;
    n->as.call.len  = tok->len;
    n->as.call.arg  = arg;
    return n;
  }

  consume(rest);
  n = new_node(ctx, NODE_VAR, tok);
  n->as.var.name = tok->loc;
  n->as.var.len  = tok->len;
  return n;
}

static Node *prefix_group(CalcCtx *ctx, Token **rest, Token *tok) {
  Node *inner;
  inner = parse_expr(ctx, rest, tok->next, 0);
  if (*rest == NULL || (*rest)->type != TK_RPAREN)
    ctx_error(ctx, "expected ')'", NULL, 0);
  consume(rest);
  return inner;
}

static Node *prefix_neg(CalcCtx *ctx, Token **rest, Token *tok) {
  Node *operand;
  Node *n;
  operand = parse_expr(ctx, rest, tok->next, 10);
  n = new_node(ctx, NODE_UNARY, tok);
  n->as.unary.op      = OP_NEG;
  n->as.unary.operand = operand;
  return n;
}

static Node *prefix_plus(CalcCtx *ctx, Token **rest, Token *tok) {
  return parse_expr(ctx, rest, tok->next, 10);
}

/* Infix handlers  */

static Node *infix_op(CalcCtx *ctx, Token **rest, Token *tok, Node *left) {
  Node *right;
  Node *n;

  if (tok->type == TK_ASSIGN) {
    if (left->kind != NODE_VAR) {
      ctx_error(ctx, "left-hand side of assignment must be a variable", tok->loc, tok->len);
    }
    right = parse_expr(ctx, rest, *rest, rules[tok->type].r_bp);
    n = new_node(ctx, NODE_ASSIGN, tok);
    n->as.assign.target = left;
    n->as.assign.value  = right;
    return n;
  }

  right = parse_expr(ctx, rest, *rest, rules[tok->type].r_bp);
  return new_binary(ctx, tok_to_op(tok->type), left, right, tok);
}

/* Rule table initialization  */

static void init_rules(void) {
  rules[TK_INT].prefix       = prefix_int;
  rules[TK_FLOAT_LIT].prefix = prefix_float;
  rules[TK_IDENT].prefix     = prefix_ident;
  rules[TK_LPAREN].prefix    = prefix_group;
  rules[TK_MINUS].prefix     = prefix_neg;
  rules[TK_PLUS].prefix      = prefix_plus;

  rules[TK_ASSIGN].l_bp = 2;
  rules[TK_ASSIGN].r_bp = 1;
  rules[TK_ASSIGN].infix = infix_op;
  rules[TK_PLUS].l_bp = 3;
  rules[TK_PLUS].r_bp = 4;
  rules[TK_PLUS].infix = infix_op;
  rules[TK_MINUS].l_bp = 3;
  rules[TK_MINUS].r_bp = 4;
  rules[TK_MINUS].infix = infix_op;
  rules[TK_MUL].l_bp = 5;
  rules[TK_MUL].r_bp = 6;
  rules[TK_MUL].infix = infix_op;
  rules[TK_DIV].l_bp = 5;
  rules[TK_DIV].r_bp = 6;
  rules[TK_DIV].infix = infix_op;
  rules[TK_POW].l_bp = 8;
  rules[TK_POW].r_bp = 7;
  rules[TK_POW].infix = infix_op;
}

/*  Pratt parser logic    */

static Node *parse_expr(CalcCtx *ctx, Token **rest, Token *tok, int min_bp) {
  Node *lhs;

  if (tok == NULL || tok->type == TK_EOF || !rules[tok->type].prefix)
    ctx_error(ctx, "expected expression", NULL, 0);

  lhs = rules[tok->type].prefix(ctx, rest, tok);

  for (;;) {
    Token *cur = *rest;
    if (cur == NULL || cur->type == TK_EOF) break;
    if (rules[cur->type].l_bp == 0 || rules[cur->type].l_bp <= min_bp) break;
    if (!rules[cur->type].infix) break;

    *rest = cur->next;
    lhs = rules[cur->type].infix(ctx, rest, cur, lhs);
  }

  return lhs;
}

/* Public functions   */

Node *parse(CalcCtx *ctx, Token *tok) {
  Node  head;
  Node *cur;

  memset(&head, 0, sizeof(head));
  cur = &head;

  init_rules();

  if (tok == NULL)
    ctx_error(ctx, "no tokens to parse", NULL, 0);

  while (tok != NULL && tok->type != TK_EOF) {
    Node *stmt = parse_expr(ctx, &tok, tok, 0);

    cur->next = stmt;
    cur = stmt;

    if (tok != NULL && tok->type == TK_SEMI) {
      tok = tok->next;
      continue;
    }

    if (tok != NULL && tok->type != TK_EOF) {
      ctx_error(ctx, "expected ';' or end of input", tok->loc, tok->len);
    }
  }

  return head.next;
}

void node_free(Node *n) {
  Node *next;
  while (n != NULL) {
    next = n->next;
    switch (n->kind) {
    case NODE_UNARY:
      node_free(n->as.unary.operand);
      break;
    case NODE_BINARY:
      node_free(n->as.binop.lhs);
      node_free(n->as.binop.rhs);
      break;
    case NODE_ASSIGN:
      node_free(n->as.assign.target);
      node_free(n->as.assign.value);
      break;
    case NODE_CALL:
      node_free(n->as.call.arg);
      break;
    default:
      break;
    }
    free(n);
    n = next;
  }
}
