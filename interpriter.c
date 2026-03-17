#include <stdio.h>
#include <stdlib.h>

#include "calc.h"

static int interpret_impl(Node *node, int *error);

static int
interpret_impl(Node *node, int *error)
{
    int rhs;

    if (*error)
        return 0;

    switch (node->type) {
    case NODE_NUM:
        return node->val;
    case NODE_ADD:
        return interpret_impl(node->lhs, error) + interpret_impl(node->rhs, error);
    case NODE_SUB:
        return interpret_impl(node->lhs, error) - interpret_impl(node->rhs, error);
    case NODE_MUL:
        return interpret_impl(node->lhs, error) * interpret_impl(node->rhs, error);
    case NODE_DIV:
        rhs = interpret_impl(node->rhs, error);
        if (*error)
            return 0;
        if (rhs == 0) {
            fprintf(stderr, "error: division by zero\n");
            *error = 1;
            return 0;
        }
        return interpret_impl(node->lhs, error) / rhs;
    default:
        fprintf(stderr, "error: unknown node type encountered\n");
        *error = 1;
        return 0;
    }
}

int
interpret(Node *node, int *out)
{
    int result;
    int error;


    error = 0;

    if (node == NULL)
        return -1; 

    result = interpret_impl(node, &error);
    if (error)
        return -1; 

    *out = result;
    return 0; 
}

