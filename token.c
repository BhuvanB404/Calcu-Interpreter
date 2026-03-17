#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calc.h"

#define MAX_INT_DIGITS 13 


typedef struct {
    const char *p; /* current lexing  position */
} SrcState;



static Token *new_token(Tokentype type, const char *loc, int len);
static Token *read_int(SrcState *st);
static void   skip_space(SrcState *st);
static void   tok_error(const char *msg);
static void   consume(SrcState *st);


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

static void skip_space(SrcState *st)
{
    while (*st->p != '\0' &&
           isspace(*st->p))
        consume(st);
}


static Token * read_int(SrcState *st)
{
    const char *start;
    int         digits;
    int         value;
    Token      *tok;

    start  = st->p;
    digits = 0;
    value  = 0;

    while (*st->p != '\0' &&
           isdigit((unsigned char)*st->p)) {
        if (digits >= MAX_INT_DIGITS) {
            tok_error("number too large");
            return NULL;
        }
        value = value * 10 + (*st->p - '0');
        consume(st);
        digits++;
    }

    tok      = new_token(TK_INT, start, (int)(st->p - start));
    tok->val = value;
    return tok;
}

/* Uses  linked list for token storage */
Token * tokenize_all(const char *src)
{
    SrcState  st;
    Token     head;  
    Token    *cur;
    char      buff[64];

    st.p = src;

    memset(&head, 0, sizeof(head));
    cur = &head;

    while (*st.p != '\0') {

        if (isspace((unsigned char)*st.p)) {
            skip_space(&st);
            continue;
        }


        if (isdigit((unsigned char)*st.p)) {
            cur = cur->next = read_int(&st);
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

/*  Free all tokens in the list */
void token_free_list(Token *tok)
{
    Token *next;

    while (tok != NULL) {
        next = tok->next;
        free(tok);
        tok = next;
    }
}