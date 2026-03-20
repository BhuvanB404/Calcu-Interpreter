#include <stdio.h>
#include <setjmp.h>

#include "calc.h"
#include "token.h"
#include "interpreter.h"

/* Public functions */

int eval(const char *src, Value *out) {
  Token *volatile toks = NULL;
  Node *volatile tree  = NULL;
  CalcCtx ctx;

  if (setjmp(ctx.error_jmp)) {
    if (ctx.error_loc && ctx.error_len > 0)
      fprintf(stderr, "Error: %s (at '%.*s')\n", ctx.error_msg, ctx.error_len, ctx.error_loc);
    else
      fprintf(stderr, "Error: %s\n", ctx.error_msg);
    if (tree) node_free(tree);
    if (toks) token_free_list(toks);
    return -1;
  }

  toks = tokenize_all(&ctx, src);
  tree = parse(&ctx, toks);
  *out = interpret(&ctx, tree);

  node_free(tree);
  token_free_list(toks);
  return 0;
}
