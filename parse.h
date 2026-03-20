#ifndef PARSE_H
#define PARSE_H

#include "token.h"

typedef enum { VAL_INT, VAL_FLOAT, VAL_COUNT } ValType;

typedef union {
    long   ival;
    double fval;
} ValAs;

typedef struct {
    ValType type;
    ValAs   as;
} Value;

typedef enum {
    NODE_LITERAL,
    NODE_VAR,
    NODE_UNARY,  /* -x, +x, x!,        */
    NODE_BINARY, /*  x+y, x**y, x&y    */
    NODE_ASSIGN, /* target = value*/
    NODE_CALL,   /* name(arg1, arg2, ...)*/
    NODE_KIND_COUNT
} NodeKind;

typedef enum {
    /* arithmetic */
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,
    /* unary */
    OP_NEG,
    OP_POS,
    OP_COUNT
} OpType;

typedef struct Node Node;

typedef struct {
    const char *name;
    int         len;
} NodeVar;

typedef struct {
    OpType op;
    Node  *operand;
} NodeUnary;

typedef struct {
    OpType op;
    Node  *lhs;
    Node  *rhs;
} NodeBinop;

typedef struct {
    Node *target;
    Node *value;
} NodeAssign;

typedef struct {
    const char *name;
    int         len;
    Node       *arg;
} NodeCall;

typedef union {
    Value      literal;
    NodeVar    var;
    NodeUnary  unary;
    NodeBinop  binop;
    NodeAssign assign;
    NodeCall   call;
} NodeAs;

struct Node {
    NodeKind kind;
    Token   *tok;  /* source location for error reporting */
    Node    *next; /* list linkage              */
    NodeAs   as;
};

Node *parse(CalcCtx *ctx, Token *tok);
void  node_free(Node *n);

#endif
