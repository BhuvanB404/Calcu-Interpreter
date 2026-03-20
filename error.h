#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>

typedef struct {
    jmp_buf     error_jmp;
    const char *error_msg;
    const char *error_loc;
    int         error_len;
    char        error_buf[64]; 
} CalcCtx;

void ctx_error(CalcCtx *ctx, const char *msg, const char *loc, int len);

#endif
