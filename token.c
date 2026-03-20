#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

#define MAX_INT_DIGITS 13

typedef struct {
    const char *p;
} SrcState;

static Token *new_token(CalcCtx *ctx, Tokentype type, const char *loc, int len);
static Token *read_number(CalcCtx *ctx, SrcState *st);
static Token *read_ident(CalcCtx *ctx, SrcState *st);
static void   skip_space(SrcState *st);
static void   consume(SrcState *st);
static int    is_ident_var(char c);
static int    is_ident_val(char c);
static Tokentype return_token_type(char ch);

/* move the cursor forward by one byte. */
static void consume(SrcState *st) {
    st->p++;
}

static Token *new_token(CalcCtx *ctx, Tokentype type, const char *loc,
                        int len) {
    Token *tok;

    tok = calloc(1, sizeof(Token));
    if (tok != NULL) {
        tok->type = type;
        tok->loc  = loc;
        tok->len  = len;
        return tok;
    }
    ctx_error(ctx, "out of memory", NULL, 0);
    return NULL; /* unreachable: ctx_error longjmps */
}

static void skip_space(SrcState *st) {
    while (*st->p != '\0' && isspace(*st->p))
        consume(st);
}

static int is_ident_var(char c) {
    return isalpha(c) || c == '_';
}

static int is_ident_val(char c) {
    return is_ident_var(c) || isdigit(c);
}

static Token *read_number(CalcCtx *ctx, SrcState *st) {
    const char *start;
    int         has_dot;
    Token      *tok;

    start   = st->p;
    has_dot = 0;

    while (*st->p != '\0' &&
           (isdigit((unsigned char)*st->p) || *st->p == '.')) {
        if (*st->p == '.') {
            if (has_dot) {
                ctx_error(ctx, "multiple decimal points in number", st->p, 1);
            }
            has_dot = 1;
        }
        consume(st);
    }

    if (has_dot) {
        tok = new_token(ctx, TK_FLOAT_LIT, start, (int)(st->p - start));
    } else {
        tok = new_token(ctx, TK_INT, start, (int)(st->p - start));
    }
    return tok;
}

static Token *read_ident(CalcCtx *ctx, SrcState *st) {
    const char *start;
    int         len;

    start = st->p;
    len   = 0;

    while (*st->p != '\0' && is_ident_val(*st->p)) {
        consume(st);
        len++;
    }

    return new_token(ctx, TK_IDENT, start, len);
}

static Tokentype return_token_type(char ch) {
    switch (ch) {
    case '+':
        return TK_PLUS;
    case '-':
        return TK_MINUS;
    case '=':
        return TK_ASSIGN;
    case ';':
        return TK_SEMI;
    case '*':
        return TK_MUL;
    case '/':
        return TK_DIV;
    case '(':
        return TK_LPAREN;
    case ')':
        return TK_RPAREN;
    default:
        return TK_ERROR;
    }
}

Token *tokenize_all(CalcCtx *ctx, const char *src) {
    SrcState  st;
    Token     head;
    Token    *cur;
    Token    *tok;
    Tokentype type;

    st.p = src;
    memset(&head, 0, sizeof(head));
    cur = &head;

    while (*st.p != '\0') {
        if (isspace((unsigned char)*st.p)) {
            skip_space(&st);
            continue;
        }

        if (isdigit((unsigned char)*st.p) || *st.p == '.') {
            tok = read_number(ctx, &st);
            cur = cur->next = tok;
            continue;
        }

        if (is_ident_var(*st.p)) {
            tok = read_ident(ctx, &st);
            cur = cur->next = tok;
            continue;
        }

        /* Two-char: ** must be checked before single * */
        if (*st.p == '*' && *(st.p + 1) == '*') {
            cur = cur->next = new_token(ctx, TK_POW, st.p, 2);
            consume(&st);
            consume(&st);
            continue;
        }

        type = return_token_type(*st.p);
        if (type != TK_ERROR) {
            cur = cur->next = new_token(ctx, type, st.p, 1);
            consume(&st);
            continue;
        }

        sprintf(ctx->error_buf, "unexpected character '%c'", *st.p);
        ctx_error(ctx, ctx->error_buf, st.p, 1);
    }

    cur->next = new_token(ctx, TK_EOF, st.p, 0);
    return head.next;
}

void token_free_list(Token *tok) {
    Token *next;

    while (tok != NULL) {
        next = tok->next;
        free(tok);
        tok = next;
    }
}