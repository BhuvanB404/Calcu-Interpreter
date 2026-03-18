#include <stdlib.h>

#include "calc.h"

int eval(const char *src, int *out) {
    Token *toks;
    Node  *tree;
    int    result;

    toks = tokenize_all(src);
    if (toks == NULL)
        return -1; /*tk error*/

    tree = parse(toks);

    if (tree == NULL) {
        token_free_list(toks);
        return -1;  /*parse error*/
    }

    result = interpret(tree, out);
    node_free(tree);
    token_free_list(toks);

    return result;
}