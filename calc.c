#include <stdlib.h>

#include "calc.h"

int eval(const char *src, int *out) {
    Token *toks;
    Node  *tree;
    int    result;

    toks = tokenize_all(src);
    if (toks == NULL)
        return -1; /* tokenization error */

    tree = parse(toks);
    token_free_list(toks);

    if (tree == NULL)
        return -1; /* parsing error */

    result = interpret(tree, out);
    node_free(tree);

    return result;
}