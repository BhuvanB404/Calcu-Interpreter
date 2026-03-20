#include <setjmp.h>

#include "error.h"

void ctx_error(CalcCtx *ctx, const char *msg, const char *loc, int len) {
    ctx->error_msg = msg;
    ctx->error_loc = loc;
    ctx->error_len = len;
    longjmp(ctx->error_jmp, 1);
}
