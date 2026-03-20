#ifndef TOKEN_H
#define TOKEN_H

#include "error.h"

typedef enum {
    TK_INT,       /* integer literal   */
    TK_FLOAT_LIT, /* floating point literal */
    TK_PLUS,      /* +                 */
    TK_MINUS,     /* -                 */
    TK_ASSIGN,    /* =                 */
    TK_IDENT,     /* identifier        */
    TK_SEMI,      /* ;                 */
    TK_MUL,       /* *                 */
    TK_DIV,       /* /                 */
    TK_LPAREN,    /* (                 */
    TK_RPAREN,    /* )                 */
    TK_POW,       /* **                */
    TK_EOF,       /* end of input      */
    TK_ERROR,     /* unrecognised char */
    TK_COUNT      /* total token types — used to size tables */
} Tokentype;

typedef struct Token Token;
struct Token {
    Tokentype   type;
    Token      *next;
    const char *loc;
    int         len;
    int         val;
};

Token *tokenize_all(CalcCtx *ctx, const char *src);
void   token_free_list(Token *tok);

#endif
