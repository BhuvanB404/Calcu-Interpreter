#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calc.h"

#define MAX_INT_DIGITS 13 


typedef struct {
    const char *p; /* current lexing  position */
} SrcState;

<<<<<<< Updated upstream


static Token *new_token(Tokentype type, const char *loc, int len);
static Token *read_int(SrcState *st);
static void   skip_space(SrcState *st);
static void   tok_error(const char *msg);
static void   consume(SrcState *st);
static int    is_ident_var(char c);
static int    is_ident_val(char c);


static void tok_error(const char *msg)
{
    fprintf(stderr, "error: %s\n", msg);
}

/* move the cursor forward by one byte.
 */
static void
consume(SrcState *st)
{
    st->p++;
}


static Token *new_token(Tokentype type, const char *loc, int len)
{
=======
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
>>>>>>> Stashed changes
    Token *tok;

    tok = calloc(1, sizeof(Token));
    if (tok == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    tok->type = type;
    tok->loc  = loc;
    tok->len  = len;
    return tok;
}

<<<<<<< Updated upstream
static void skip_space(SrcState *st)
{
    while (*st->p != '\0' &&
           isspace(*st->p))
=======
static void skip_space(SrcState *st) {
    while (*st->p != '\0' && isspace(*st->p))
>>>>>>> Stashed changes
        consume(st);
}

static int is_ident_var(char c) {
    return isalpha(c) || c == '_';
}

static int is_ident_val(char c) {
    return is_ident_var(c) || isdigit(c);
}

<<<<<<< Updated upstream
static Token * read_int(SrcState *st)
{
=======
static Token *read_number(CalcCtx *ctx, SrcState *st) {
>>>>>>> Stashed changes
    const char *start;
    int         has_dot;
    Token      *tok;

    start   = st->p;
    has_dot = 0;

    while (*st->p != '\0' &&
<<<<<<< Updated upstream
           isdigit((unsigned char)*st->p)) {
        if (digits >= MAX_INT_DIGITS) {
            tok_error("number too large");
            return NULL;
        }
        value = value * 10 + (*st->p - '0');
=======
           (isdigit((unsigned char)*st->p) || *st->p == '.')) {
        if (*st->p == '.') {
            if (has_dot) {
                ctx_error(ctx, "multiple decimal points in number", st->p, 1);
            }
            has_dot = 1;
        }
>>>>>>> Stashed changes
        consume(st);
    }

<<<<<<< Updated upstream
    tok      = new_token(TK_INT, start, (int)(st->p - start));
    tok->val = value;
    return tok;
}

static Token* read_ident(SrcState *st)
{
=======
    if (has_dot) {
        tok = new_token(ctx, TK_FLOAT_LIT, start, (int)(st->p - start));
    } else {
        tok = new_token(ctx, TK_INT, start, (int)(st->p - start));
    }
    return tok;
}

static Token *read_ident(CalcCtx *ctx, SrcState *st) {
>>>>>>> Stashed changes
    const char *start;
    int         len;
    Token      *tok;

    start = st->p;
    len   = 0;

    if (!is_ident_var(*st->p)) {
        tok_error("invalid identifier");
        return NULL;
    }

    while (*st->p != '\0' &&
           is_ident_val(*st->p)) {
        consume(st);
        len++;
    }

    tok = new_token(TK_IDENT, start, len);
    return tok;
}

<<<<<<< Updated upstream


/* Uses  linked list for token storage */
Token * tokenize_all(const char *src)
{
=======
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
>>>>>>> Stashed changes
    SrcState  st;
    Token     head;  
    Token    *cur;
    Token    *tok;
    char      buff[64];

    st.p = src;

    memset(&head, 0, sizeof(head));
    cur = &head;

    while (*st.p != '\0') {

        if (isspace((unsigned char)*st.p)) {
            skip_space(&st);
            continue;
        }

<<<<<<< Updated upstream

        if (isdigit((unsigned char)*st.p)) {
            tok = read_int(&st);
            if (tok == NULL) {
                token_free_list(head.next);
                return NULL;
            }
=======
        if (isdigit((unsigned char)*st.p) || *st.p == '.') {
            tok = read_number(ctx, &st);
>>>>>>> Stashed changes
            cur = cur->next = tok;
            continue;
        }

        if(is_ident_var(*st.p)) {
            tok = read_ident(&st);
            if (tok == NULL) {
                token_free_list(head.next);
                return NULL;
            }
            cur = cur->next = tok;
            continue;
        }

        switch (*st.p) {
        case '+':
            cur = cur->next = new_token(TK_PLUS, st.p, 1);
            consume(&st);
            continue;
        case '-':
            cur = cur->next = new_token(TK_MINUS, st.p, 1);
            consume(&st);
            continue;
        case '=':
            cur = cur->next = new_token(TK_ASSIGN, st.p, 1);
            consume(&st);
            continue;
        case ';':
            cur = cur->next = new_token(TK_SEMI, st.p, 1);
            consume(&st);
            continue;
        case '*':
            cur = cur->next = new_token(TK_MUL, st.p, 1);
            consume(&st);
            continue;
        case '/':
            cur = cur->next = new_token(TK_DIV, st.p, 1);
            consume(&st);
            continue;
        case '(':
            cur = cur->next = new_token(TK_LPAREN, st.p, 1);
            consume(&st);
            continue;
        case ')':
            cur = cur->next = new_token(TK_RPAREN, st.p, 1);
            consume(&st);
            continue;
        default:
            break;
        }

        sprintf(buff, "unexpected character '%c'", *st.p);
        tok_error(buff);
        token_free_list(head.next);
        return NULL; 
    }

 
    cur->next = new_token(TK_EOF, st.p, 0);

    return head.next; 
}

<<<<<<< Updated upstream
/*  Free all tokens in the list */
void token_free_list(Token *tok)
{
=======
void token_free_list(Token *tok) {
>>>>>>> Stashed changes
    Token *next;

    while (tok != NULL) {
        next = tok->next;
        free(tok);
        tok = next;
    }
}