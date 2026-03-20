#include <stdlib.h>

#include "calc.h"
<<<<<<< Updated upstream

int eval(const char *src, int *out) {
    Token *toks;
    Node  *tree;
    int    result;
=======
#include "token.h"
#include "parse.h"
#include "interpreter.h"

static void report_error(CalcCtx *ctx) {
    if (ctx->error_loc && ctx->error_len > 0) {
        fprintf(stderr, "Error: %s (at '%.*s')\n", ctx->error_msg,
                ctx->error_len, ctx->error_loc);
    } else {
        fprintf(stderr, "Error: %s\n", ctx->error_msg);
    }
}

int eval(const char *src, Value *out) {
    Token *volatile toks = NULL;
    Node *volatile tree  = NULL;
    CalcCtx ctx;
>>>>>>> Stashed changes

    toks = tokenize_all(src);
    if (toks == NULL)
        return -1; /*tk error*/

    tree = parse(toks);

    if (tree == NULL) {
        token_free_list(toks);
        return -1;  /*parse error*/
    }

<<<<<<< Updated upstream
    result = interpret(tree, out);
=======
    toks = tokenize_all(&ctx, src);
    tree = parse(&ctx, toks);
    *out = interpret(&ctx, tree);

>>>>>>> Stashed changes
    node_free(tree);
    token_free_list(toks);

    return result;
}