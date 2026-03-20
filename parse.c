#include <stdio.h>
#include <stdlib.h>
<<<<<<< Updated upstream

#include "calc.h"

static Node *new_node(NodeType type, Token *tok);
static Node *new_binary(NodeType type, Node *lhs,
                        Node *rhs, Token *tok);
static Node *new_num(int val, Token *tok);
static int operator_prec(Tokentype type);
static Node *new_var(Token *tok);
static NodeType token_to_node(Tokentype type);
static Node *parse_expr(Token **rest, Token *tok,int min_prec);
static Node *parse_primary(Token **rest, Token *tok);
static Node *parse_unary(Token **rest, Token *tok);
static Node *parse_assign(Token **rest, Token *tok);

static Node * new_node(NodeType type, Token *tok)
{
    Node *n;

    n = calloc(1, sizeof(Node));
    if (n == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    n->type = type;
    n->tok  = tok;
    return n;
}

static Node * new_binary(NodeType type, Node *lhs, Node *rhs, Token *tok)
{
    Node *n;

         n = new_node(type, tok);
    n->lhs = lhs;
    n->rhs = rhs;
    return n;
}

static Node * new_num(int val, Token *tok)
{
    Node *n;

    n      = new_node(NODE_NUM, tok);
    n->val = val;
    return n;
}

static Node * new_var(Token *tok)
{
    Node *n;

    n = new_node(NODE_VAR, tok);
    return n;
}

static int operator_prec(Tokentype type)
{
    switch (type) {
    case TK_MUL:
    case TK_DIV:   return 2;
    case TK_PLUS:
    case TK_MINUS: return 1;
    default:       return 0;
    }
}

static NodeType token_to_node(Tokentype type)
{
    switch (type) {
    case TK_PLUS:  return NODE_ADD;
    case TK_MINUS: return NODE_SUB;
    case TK_MUL:   return NODE_MUL;
    case TK_DIV:   return NODE_DIV;
    default:       return NODE_NUM; /* unreachable */
    }
}

static Node * parse_primary(Token **rest, Token *tok)
{
    Node *n;

    if (tok == NULL) {
        fprintf(stderr, "error: unexpected  input parse error\n");
        return NULL;
    }

    if (tok->type == TK_INT) {
        *rest = tok->next;
        return new_num(tok->val, tok);
    }

    if(tok->type == TK_IDENT) {
        *rest = tok->next;
        return new_var(tok);
    }

    if (tok->type == TK_LPAREN) {
        n = parse_assign(rest, tok->next);
        if (n == NULL || (*rest)->type != TK_RPAREN) {
            fprintf(stderr, "error: expected ')'\n");
            if (n != NULL)
                node_free(n);
            return NULL;
        }
        *rest = (*rest)->next;
        return n;
    }

    fprintf(stderr, "error: expected number or '('\n");
    return NULL;
}

static Node *parse_unary(Token **rest, Token *tok)
{
    Node *operand;

    if (tok == NULL) {
        fprintf(stderr, "error: unexpected end of input\n");
        return NULL;
    }

    if (tok->type == TK_PLUS)
        return parse_unary(rest, tok->next);

    if (tok->type == TK_MINUS) {
        operand = parse_unary(rest, tok->next);
        if (operand == NULL)
            return NULL;
        return new_binary(NODE_SUB, new_num(0, tok),
                          operand, tok);
    }
    return parse_primary(rest, tok);
}


static Node *parse_assign(Token **rest, Token *tok)
{
    Node *lhs;
    Node *rhs;
    Token *og_tok;

    lhs = parse_expr(&tok, tok, 0);
    if (lhs == NULL)
        return NULL;

    if (tok != NULL && tok->type == TK_ASSIGN) {
        if (lhs->type != NODE_VAR) {
            fprintf(stderr, "error: lhs of assignment must be a variable\n");
            node_free(lhs);
            return NULL;
        }

        og_tok = tok;
        rhs = parse_assign(&tok, tok->next);
        if (rhs == NULL) {
            node_free(lhs);
            return NULL;
        }

        *rest = tok;
        return new_binary(NODE_ASSIGN, lhs, rhs, og_tok);
    }

    *rest = tok;
    return lhs;
}
static Node *parse_expr(Token **rest, Token *tok, int min_prec)
{
    Node    *lhs;
    Node    *rhs;
    Token   *og_tok;
    NodeType type;
    int      prec;

    lhs = parse_unary(&tok, tok);
    if (lhs == NULL)
        return NULL;

    for (;;) {
        if (tok == NULL) {
            fprintf(stderr, "error: unexpected end of input\n");
            node_free(lhs);
            return NULL;
        }
        prec = operator_prec(tok->type);
        if (prec == 0 || prec < min_prec)
            break;

        og_tok = tok;
        tok    = tok->next;
        rhs    = parse_expr(&tok, tok, prec + 1);
        if (rhs == NULL) {
            node_free(lhs);
            return NULL;
        }
        type   = token_to_node(og_tok->type);
        lhs    = new_binary(type, lhs, rhs, og_tok);
    }

    *rest = tok;
    return lhs;
}

Node *parse(Token *tok)
{
    Node head = {0};
    Node *cur;
    Node *stmt;

    if (tok == NULL) {
        fprintf(stderr, "error: no tokens to parse\n");
        return NULL;
    }

    cur = &head;

    while (tok != NULL && tok->type != TK_EOF) {
        stmt = parse_assign(&tok, tok);
        if (stmt == NULL) {
            node_free(head.next);
            return NULL;
        }

        cur->next = stmt;
        cur = stmt;

        if (tok != NULL && tok->type == TK_SEMI) {
            tok = tok->next;
            continue;
        }

        if (tok != NULL && tok->type != TK_EOF) {
            fprintf(stderr,
                    "error: expected ';' or end of input\n");
            node_free(head.next);
            return NULL;
        }
    }

    return head.next;
}


void node_free(Node *n)
{
    if (n == NULL)
        return;
    node_free(n->next);
    node_free(n->lhs);
    node_free(n->rhs);
    free(n);
=======
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

static ParseRule rules[TK_COUNT];

static Node *parse_expr(CalcCtx *ctx, Token **rest, Token *tok, int min_bp);

static void consume(Token **rest) {
    *rest = (*rest)->next;
}

static Node *new_node(CalcCtx *ctx, NodeKind kind, Token *tok) {
    Node *n = calloc(1, sizeof(Node));
    if (n != NULL) {
        n->kind = kind;
        n->tok  = tok;
        return n;
    }
    ctx_error(ctx, "out of memory", NULL, 0);
    return NULL; /* unreachable */
}

static Node *new_binary(CalcCtx *ctx, OpType op, Node *lhs, Node *rhs,
                        Token *tok) {
    Node *n         = new_node(ctx, NODE_BINARY, tok);
    n->as.binop.op  = op;
    n->as.binop.lhs = lhs;
    n->as.binop.rhs = rhs;
    return n;
}

static Node *new_literal_int(CalcCtx *ctx, long val, Token *tok) {
    Node *n               = new_node(ctx, NODE_LITERAL, tok);
    n->as.literal.type    = VAL_INT;
    n->as.literal.as.ival = val;
    return n;
}

static Node *new_literal_float(CalcCtx *ctx, double val, Token *tok) {
    Node *n               = new_node(ctx, NODE_LITERAL, tok);
    n->as.literal.type    = VAL_FLOAT;
    n->as.literal.as.fval = val;
    return n;
}

static OpType tok_to_op(Tokentype t) {
    switch (t) {
    case TK_PLUS:
        return OP_ADD;
    case TK_MINUS:
        return OP_SUB;
    case TK_MUL:
        return OP_MUL;
    case TK_DIV:
        return OP_DIV;
    case TK_POW:
        return OP_POW;
    default:
        return OP_ADD;
    }
}

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

    /* handles funcion calls such as sin(1)*/
    if (next != NULL && next->type == TK_LPAREN) {
        *rest = next->next;
        arg   = parse_expr(ctx, rest, *rest, 0);
        if (*rest == NULL || (*rest)->type != TK_RPAREN)
            ctx_error(ctx, "expected ')' after function argument", NULL, 0);

        consume(rest);

        n               = new_node(ctx, NODE_CALL, tok);
        n->as.call.name = tok->loc;
        n->as.call.len  = tok->len;
        n->as.call.arg  = arg;
        return n;
    }
    /* handles normal variables */
    consume(rest);
    n              = new_node(ctx, NODE_VAR, tok);
    n->as.var.name = tok->loc;
    n->as.var.len  = tok->len;
    return n;
}

/*Handles nested expressions */
static Node *prefix_group(CalcCtx *ctx, Token **rest, Token *tok) {
    Node *inner;
    inner = parse_expr(ctx, rest, tok->next, 0);
    if (*rest == NULL || (*rest)->type != TK_RPAREN)
        ctx_error(ctx, "expected ')'", NULL, 0);
    consume(rest);
    return inner;
}

/* these two handle postive and negative numbers */
static Node *prefix_neg(CalcCtx *ctx, Token **rest, Token *tok) {
    Node *operand;
    Node *n;
    operand             = parse_expr(ctx, rest, tok->next, 10);
    n                   = new_node(ctx, NODE_UNARY, tok);
    n->as.unary.op      = OP_NEG;
    n->as.unary.operand = operand;
    return n;
}

static Node *prefix_plus(CalcCtx *ctx, Token **rest, Token *tok) {
    return parse_expr(ctx, rest, tok->next, 10);
}

static Node *infix_op(CalcCtx *ctx, Token **rest, Token *tok, Node *left) {
    Node *right;
    Node *n;

    if (tok->type == TK_ASSIGN) {
        if (left->kind != NODE_VAR) {
            ctx_error(ctx, "left-hand side of assignment must be a variable",
                      tok->loc, tok->len);
        }
        right = parse_expr(ctx, rest, *rest, rules[tok->type].r_bp);
        n     = new_node(ctx, NODE_ASSIGN, tok);
        n->as.assign.target = left;
        n->as.assign.value  = right;
        return n;
    }

    right = parse_expr(ctx, rest, *rest, rules[tok->type].r_bp);
    return new_binary(ctx, tok_to_op(tok->type), left, right, tok);
}

/* functional relation table */

static void init(void) {
    rules[TK_INT].prefix       = prefix_int;
    rules[TK_FLOAT_LIT].prefix = prefix_float;
    rules[TK_IDENT].prefix     = prefix_ident;
    rules[TK_LPAREN].prefix    = prefix_group;
    rules[TK_MINUS].prefix     = prefix_neg;
    rules[TK_PLUS].prefix      = prefix_plus;

    rules[TK_ASSIGN].l_bp  = 2;
    rules[TK_ASSIGN].r_bp  = 1;
    rules[TK_ASSIGN].infix = infix_op;
    rules[TK_PLUS].l_bp    = 3;
    rules[TK_PLUS].r_bp    = 4;
    rules[TK_PLUS].infix   = infix_op;
    rules[TK_MINUS].l_bp   = 3;
    rules[TK_MINUS].r_bp   = 4;
    rules[TK_MINUS].infix  = infix_op;
    rules[TK_MUL].l_bp     = 5;
    rules[TK_MUL].r_bp     = 6;
    rules[TK_MUL].infix    = infix_op;
    rules[TK_DIV].l_bp     = 5;
    rules[TK_DIV].r_bp     = 6;
    rules[TK_DIV].infix    = infix_op;
    rules[TK_POW].l_bp     = 8;
    rules[TK_POW].r_bp     = 7;
    rules[TK_POW].infix    = infix_op;
}

static Node *parse_expr(CalcCtx *ctx, Token **rest, Token *tok, int min_bp) {
    Node *lhs;

    if (tok == NULL || tok->type == TK_EOF || !rules[tok->type].prefix)
        ctx_error(ctx, "expected expression", NULL, 0);

    lhs = rules[tok->type].prefix(ctx, rest, tok);

    for (;;) {
        Token *cur = *rest;
        if (cur == NULL || cur->type == TK_EOF)
            break;
        if (rules[cur->type].l_bp == 0 || rules[cur->type].l_bp <= min_bp)
            break;
        if (!rules[cur->type].infix)
            break;

        *rest = cur->next;
        lhs   = rules[cur->type].infix(ctx, rest, cur, lhs);
    }

    return lhs;
}

Node *parse(CalcCtx *ctx, Token *tok) {
    Node  head;
    Node *cur;

    memset(&head, 0, sizeof(head));
    cur = &head;

    init();

    if (tok == NULL)
        ctx_error(ctx, "no tokens to parse", NULL, 0);

    while (tok != NULL && tok->type != TK_EOF) {
        Node *stmt = parse_expr(ctx, &tok, tok, 0);

        cur->next = stmt;
        cur       = stmt;

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
>>>>>>> Stashed changes
}