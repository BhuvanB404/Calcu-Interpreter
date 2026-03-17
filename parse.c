#include <stdio.h>
#include <stdlib.h>

#include "calc.h"

static Node *new_node(NodeType type, Token *tok);
static Node *new_binary(NodeType type, Node *lhs,
                        Node *rhs, Token *tok);
static Node *new_num(int val, Token *tok);
static int   operator_prec(Tokentype type);
static NodeType token_to_node(Tokentype type);
static Node *parse_expr(Token **rest, Token *tok,int min_prec);
static Node *parse_primary(Token **rest, Token *tok);
static Node *parse_unary(Token **rest, Token *tok);


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

    if (tok->type == TK_LPAREN) {
        n = parse_expr(rest, tok->next, 0);
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

static Node *parse_expr(Token **rest, Token *tok, int min_prec)
{
    Node    *lhs;
    Node    *rhs;
    Token   *op_tok;
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

        op_tok = tok;
        tok    = tok->next;
        rhs    = parse_expr(&tok, tok, prec + 1);
        if (rhs == NULL) {
            node_free(lhs);
            return NULL;
        }
        type   = token_to_node(op_tok->type);
        lhs    = new_binary(type, lhs, rhs, op_tok);
    }

    *rest = tok;
    return lhs;
}

Node *parse(Token *tok)
{
    Node *tree;

    if (tok == NULL) {
        fprintf(stderr, "error: no tokens to parse\n");
        return NULL;
    }

    tree = parse_expr(&tok, tok, 0);
    if (tree == NULL)
        return NULL;

    if (tok == NULL || tok->type != TK_EOF) {
        fprintf(stderr,
                "error: unexpected input after expression\n");
        node_free(tree);
        return NULL;
    }

    return tree;
}


void node_free(Node *n)
{
    if (n == NULL)
        return;
    node_free(n->lhs);
    node_free(n->rhs);
    free(n);
}