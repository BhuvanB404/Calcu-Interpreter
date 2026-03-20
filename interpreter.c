#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "calc.h"

/* structs */

typedef struct Var Var;
struct Var {
  const char *name;
  int         len;
  Value       val;
  Var        *next;
};

typedef Value (*node_func)(CalcCtx *, Node *);
typedef Value (*binary_func)(CalcCtx *, Value, Value);
typedef Value (*unary_func)(CalcCtx *, Value);
typedef Value (*math_func)(Value *);

typedef struct {
  const char *name;
  int         len;
  math_func   fn;
} FuncEntry;

/*      function declataions */

static Value eval_node(CalcCtx *ctx, Node *node);
static Value eval_literal(CalcCtx *ctx, Node *node);
static Value eval_var(CalcCtx *ctx, Node *node);
static Value eval_unary(CalcCtx *ctx, Node *node);
static Value eval_binary(CalcCtx *ctx, Node *node);
static Value eval_assign(CalcCtx *ctx, Node *node);
static Value eval_call(CalcCtx *ctx, Node *node);

/*  tables  */

static node_func   node_table[NODE_KIND_COUNT];
static binary_func bin_int_table[OP_COUNT];
static binary_func bin_float_table[OP_COUNT];
static unary_func  un_int_table[OP_COUNT];
static unary_func  un_float_table[OP_COUNT];
static Var        *var_list;

/* initialisation functions  */

static Value val_int(long v) {
  Value r;
  r.type    = VAL_INT;
  r.as.ival = v;
  return r;
}

static Value val_float(double v) {
  Value r;
  r.type    = VAL_FLOAT;
  r.as.fval = v;
  return r;
}

static Value to_float(Value v) {
  if (v.type == VAL_INT)
    return val_float((double)v.as.ival);
  return v;
}

static void promote(Value *l, Value *r) {
  if (l->type == r->type)
    return;
  if (l->type == VAL_FLOAT) {
    *r = to_float(*r);
    return;
  }
  if (r->type == VAL_FLOAT)
    *l = to_float(*l);
}

/*INT   Binary functions */

static Value bin_add_int(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_int(l.as.ival + r.as.ival);
}

static Value bin_sub_int(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_int(l.as.ival - r.as.ival);
}

static Value bin_mul_int(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_int(l.as.ival * r.as.ival);
}

static Value bin_div_int(CalcCtx *ctx, Value l, Value r) {
  if (r.as.ival == 0)
    ctx_error(ctx, "division by zero", NULL, 0);
  return val_int(l.as.ival / r.as.ival);
}

static Value bin_mod_int(CalcCtx *ctx, Value l, Value r) {
  if (r.as.ival == 0)
    ctx_error(ctx, "modulo by zero", NULL, 0);
  return val_int(l.as.ival % r.as.ival);
}

static Value bin_pow_int(CalcCtx *ctx, Value l, Value r) {
  long result;
  long i;
  if (r.as.ival < 0)
    ctx_error(ctx, "negative exponent on integer", NULL, 0);
  result = 1;
  for (i = 0; i < r.as.ival; i++)
    result *= l.as.ival;
  return val_int(result);
}


/*FLOAT Binary functions */


static Value bin_add_float(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_float(l.as.fval + r.as.fval);
}

static Value bin_sub_float(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_float(l.as.fval - r.as.fval);
}

static Value bin_mul_float(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_float(l.as.fval * r.as.fval);
}

static Value bin_div_float(CalcCtx *ctx, Value l, Value r) {
  if (r.as.fval == 0.0)
    ctx_error(ctx, "division by zero", NULL, 0);
  return val_float(l.as.fval / r.as.fval);
}

static Value bin_mod_float(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_float(fmod(l.as.fval, r.as.fval));
}

static Value bin_pow_float(CalcCtx *ctx, Value l, Value r) {
  (void)ctx;
  return val_float(pow(l.as.fval, r.as.fval));
}

/* Unary operations  */

static Value un_neg_int(CalcCtx *ctx, Value v) {
  (void)ctx;
  return val_int(-v.as.ival);
}

static Value un_neg_float(CalcCtx *ctx, Value v) {
  (void)ctx;
  return val_float(-v.as.fval);
}

static Value un_pos_int(CalcCtx *ctx, Value v) {
  (void)ctx;
  return val_int(+v.as.ival);
}

static Value un_pos_float(CalcCtx *ctx, Value v) {
  (void)ctx;
  return val_float(+v.as.fval);
}

/* Math Functions  */

static Value f_sin(Value *a) {
  return val_float(sin(to_float(a[0]).as.fval));
}

static Value f_cos(Value *a) {
  return val_float(cos(to_float(a[0]).as.fval));
}

static Value f_tan(Value *a) {
  return val_float(tan(to_float(a[0]).as.fval));
}

static Value f_log(Value *a) {
  return val_float(log(to_float(a[0]).as.fval));
}

static const FuncEntry func_table[] = {
  {"sin", 3, f_sin},
  {"cos", 3, f_cos},
  {"tan", 3, f_tan},
  {"log", 3, f_log}
};

/* Variable manupilation  */

static Var *find_var(const char *name, int len) {
  Var *v;
  for (v = var_list; v; v = v->next)
    if (v->len == len && strncmp(v->name, name, (size_t)len) == 0)
      return v;
  return NULL;
}

static int set_var(const char *name, int len, Value val) {
  Var *v;

  v = find_var(name, len);
  if (v) {
    v->val = val;
    return 0;
  }

  v = malloc(sizeof(Var));
  if (!v)
    return -1;

  v->name  = name;
  v->len   = len;
  v->val   = val;
  v->next  = var_list;
  var_list = v;
  return 0;
}

static int get_var(const char *name, int len, Value *out) {
  Var *v;
  v = find_var(name, len);
  if (!v)
    return -1;
  *out = v->val;
  return 0;
}

/*  Table initialisation */

static void init_tables(void) {
  node_table[NODE_LITERAL] = eval_literal;
  node_table[NODE_VAR]     = eval_var;
  node_table[NODE_UNARY]   = eval_unary;
  node_table[NODE_BINARY]  = eval_binary;
  node_table[NODE_ASSIGN]  = eval_assign;
  node_table[NODE_CALL]    = eval_call;

  bin_int_table[OP_ADD] = bin_add_int;
  bin_int_table[OP_SUB] = bin_sub_int;
  bin_int_table[OP_MUL] = bin_mul_int;
  bin_int_table[OP_DIV] = bin_div_int;
  bin_int_table[OP_MOD] = bin_mod_int;
  bin_int_table[OP_POW] = bin_pow_int;

  bin_float_table[OP_ADD] = bin_add_float;
  bin_float_table[OP_SUB] = bin_sub_float;
  bin_float_table[OP_MUL] = bin_mul_float;
  bin_float_table[OP_DIV] = bin_div_float;
  bin_float_table[OP_MOD] = bin_mod_float;
  bin_float_table[OP_POW] = bin_pow_float;

  un_int_table[OP_NEG] = un_neg_int;
  un_int_table[OP_POS] = un_pos_int;

  un_float_table[OP_NEG] = un_neg_float;
  un_float_table[OP_POS] = un_pos_float;
}

/* Node evaluation functions  */

static Value eval_literal(CalcCtx *ctx, Node *node) {
  (void)ctx;
  return node->as.literal;
}

static Value eval_var(CalcCtx *ctx, Node *node) {
  Value out;
  if (get_var(node->as.var.name, node->as.var.len, &out) != 0)
    ctx_error(ctx, "undefined variable", node->tok->loc, node->tok->len);
  return out;
}

static Value eval_unary(CalcCtx *ctx, Node *node) {
  Value      v;
  int        op;
  unary_func f;

  v  = eval_node(ctx, node->as.unary.operand);
  op = (int)node->as.unary.op;

  if (op < 0 || op >= OP_COUNT)
    ctx_error(ctx, "unsupported unary operation", node->tok->loc, node->tok->len);

  if (v.type == VAL_INT)
    f = un_int_table[op];
  else if (v.type == VAL_FLOAT)
    f = un_float_table[op];
  else
    f = NULL;

  if (!f)
    ctx_error(ctx, "unsupported unary operation", node->tok->loc, node->tok->len);
  return f(ctx, v);
}

static Value eval_binary(CalcCtx *ctx, Node *node) {
  Value       lhs;
  Value       rhs;
  int         op;
  binary_func f;

  lhs = eval_node(ctx, node->as.binop.lhs);
  rhs = eval_node(ctx, node->as.binop.rhs);
  op  = (int)node->as.binop.op;

  promote(&lhs, &rhs);

  if (op < 0 || op >= OP_COUNT)
    ctx_error(ctx, "unsupported binary operation", node->tok->loc, node->tok->len);

  if (lhs.type == VAL_INT)
    f = bin_int_table[op];
  else if (lhs.type == VAL_FLOAT)
    f = bin_float_table[op];
  else
    f = NULL;

  if (!f)
    ctx_error(ctx, "unsupported binary operation", node->tok->loc, node->tok->len);
  return f(ctx, lhs, rhs);
}

static Value eval_assign(CalcCtx *ctx, Node *node) {
  Value rhs;
  Node *tgt;

  rhs = eval_node(ctx, node->as.assign.value);
  tgt = node->as.assign.target;

  if (set_var(tgt->as.var.name, tgt->as.var.len, rhs) != 0)
    ctx_error(ctx, "out of memory", node->tok->loc, node->tok->len);
  return rhs;
}

static Value eval_call(CalcCtx *ctx, Node *node) {
  NodeCall *call;
  Value     arg;
  int       i;
  int       n;

  call = &node->as.call;
  n    = sizeof(func_table) / sizeof(func_table[0]);

  for (i = 0; i < n; i++) {
    if (func_table[i].len == call->len &&
        strncmp(func_table[i].name, call->name, (size_t)call->len) == 0) {
      arg = eval_node(ctx, call->arg);
      return func_table[i].fn(&arg);
    }
  }

  ctx_error(ctx, "unknown function", call->name, call->len);
  return val_int(0);
}

static Value eval_node(CalcCtx *ctx, Node *node) {
  node_func f;

  if (!node)
    ctx_error(ctx, "unexpected NULL node", NULL, 0);

  f = node_table[node->kind];
  if (!f)
    ctx_error(ctx, "unknown node kind", node->tok->loc, node->tok->len);
  return f(ctx, node);
}

/*  Public functions*/

Value interpret(CalcCtx *ctx, Node *node) {
  Value result;
  Node *cur;

  init_tables();

  result = val_int(0);

  if (!node)
    ctx_error(ctx, "no AST to evaluate", NULL, 0);

  for (cur = node; cur; cur = cur->next)
    result = eval_node(ctx, cur);

  return result;
}

void public_set_var(const char *name, int len, Value val) {
  set_var(name, len, val);
}
