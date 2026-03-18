#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calc.h"

typedef struct Var Var;
 struct Var {

    char *name;
    int   val;
    Var  *next;
} ;


static Var *var_list;

static int interpret_impl(Node *node, int *error);
static Var *find_var(const char *name, int len);
static Var *new_var(const char *name, int val);
static char *dup_string(const char *name);
static int set_var(const char *name, int len, int val);
static int get_var(const char *name, int len, int *out);
static int token_name(Token *tok, const char **name, int *len);

static char *dup_string(const char *name)
{
    size_t len;
    char *copy;

    len = strlen(name);
    copy = malloc(len + 1);
    if (copy == NULL)
        return NULL;

    memcpy(copy, name, len + 1);
    return copy;
}



static int token_name(Token *tok, const char **name, int *len)
{
    if (tok == NULL || tok->type != TK_IDENT) {
        fprintf(stderr, "error: invalid variable token\n");
        return -1;
    }

    *name = tok->loc;
    *len = tok->len;
    return 0;
}

static Var *find_var(const char *name, int len)
{
    Var *v;

    for (v = var_list; v != NULL; v = v->next) {
        if ((int)strlen(v->name) == len &&
            strncmp(v->name, name, (size_t)len) == 0)
            return v;
    }
    return NULL;
}

static Var *new_var(const char *name, int val)
{
    Var *v;

    v = malloc(sizeof(Var));
    if (v == NULL)
        return NULL;

    v->name = dup_string(name);
    if (v->name == NULL) {
        free(v);
        return NULL;
    }

    v->val  = val;
    v->next = var_list;
    var_list = v;
    return v;
}

static int set_var(const char *name, int len, int val)
{
    Var *v;
    char *tmp;

    v = find_var(name, len);
    if (v == NULL) {
        tmp = malloc((size_t)len + 1);
        if (tmp == NULL)
            return -1;

        memcpy(tmp, name, (size_t)len);
        tmp[len] = '\0';

        v = new_var(tmp, val);
        free(tmp);
        if (v == NULL)
            return -1;
        return 0;
    }

    v->val = val;
    return 0;
}

static int get_var(const char *name, int len, int *out)
{
    Var *v;
    char *tmp;

    tmp = malloc((size_t)len + 1);
    if (tmp == NULL) {
        fprintf(stderr, "error: out of memory\n");
        return -1;
    }

    memcpy(tmp, name, (size_t)len);
    tmp[len] = '\0';

    v = find_var(name, len);
    if (v == NULL) {
        fprintf(stderr, "error: undefined variable '%s'\n", tmp);
        free(tmp);
        return -1;
    }

    *out = v->val;
    free(tmp);
    return 0;
}

static int interpret_impl(Node *node, int *error)
{
    const char *name;
    int name_len;
    int lhs;
    int rhs;

    if (*error)
        return 0;

    switch (node->type) {
    case NODE_NUM:
        return node->val;
    case NODE_VAR:
        if (token_name(node->tok, &name, &name_len) != 0) {
            *error = 1;
            return 0;
        }
        if (get_var(name, name_len, &rhs) != 0) {
            *error = 1;
            return 0;
        }
        return rhs;
    case NODE_ASSIGN:
        if (node->lhs == NULL || node->lhs->type != NODE_VAR) {
            fprintf(stderr, "error: lhs of assignment must be a variable\n");
            *error = 1;
            return 0;
        }

        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;

        if (token_name(node->lhs->tok, &name, &name_len) != 0) {
            *error = 1;
            return 0;
        }

        if (set_var(name, name_len, rhs) != 0) {
            fprintf(stderr, "error: out of memory\n");
            *error = 1;
            return 0;
        }
        return rhs; 
    case NODE_ADD:
        lhs = interpret_impl(node->lhs, error);
        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;
        return lhs + rhs;
    case NODE_SUB:
        lhs = interpret_impl(node->lhs, error);
        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;
        return lhs - rhs;
    case NODE_MUL:
        lhs = interpret_impl(node->lhs, error);
        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;
        return lhs * rhs;
    case NODE_DIV:
        lhs = interpret_impl(node->lhs, error);
        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;
        if (rhs == 0) {
            fprintf(stderr, "error: division by zero\n");
            *error = 1;
            return 0;
        }
        return lhs / rhs;
    default:
        fprintf(stderr, "error: unknown node type encountered\n");
        *error = 1;
        return 0;
    }
}

int interpret(Node *node, int *out)
{
    int value;
    int error;
    Node *cur;


    error = 0;

    if (node == NULL)
        return -1; 

    cur = node;
    value = 0;
    while (cur != NULL) {
        value = interpret_impl(cur, &error);
        if (error)
            return -1;
        cur = cur->next;
    }

    *out = value;
    return 0; 
}

