

typedef enum {
    TK_INT,     /* integer literal   */
    TK_PLUS,    /* +                 */
    TK_MINUS,   /* -                 */
    TK_ASSIGN,   /* =                 */
    TK_IDENT,   /* identifier        */
    TK_SEMI,    /* ;                 */
    TK_MUL,     /* *                 */
    TK_DIV,     /* /                 */
    TK_LPAREN,  /* (                 */
    TK_RPAREN,  /* )                 */
    TK_EOF,     /* end of input      */
    TK_ERROR    /* unrecognised char */
} Tokentype;

typedef struct Token Token;
struct Token {
    Tokentype   type;  /* what type of token            */
    Token      *next;  /* pointing to next token in the linked list */
    const char *loc;   /* pointer into source string    */
    int         len;   /* byte length of this token     */

    int         val;   /* integer value if TK_INT       */
};

typedef enum {
    NODE_NUM,  /* int leaf    */
    NODE_ADD,  /*lhs + rhs    */
    NODE_VAR,  /* variable     */
    NODE_ASSIGN, /* lhs = rhs ; var = int   */
    NODE_SUB,  /* lhs - rhs    */
    NODE_MUL,  /* lhs * rhs    */
    NODE_DIV   /*lhs / rhs    */
} NodeType;

typedef struct Node Node;
struct Node {
    NodeType  type;  
    Token    *tok;   
    Node     *lhs;   
    Node     *rhs;   
    Node     *next;  /* next statement in a line */
    int       val;   
};


Token *tokenize_all(const char *src);


void token_free_list(Token *tok);

Node *parse(Token *tok);


void node_free(Node *n);

int interpret(Node *n, int *out);


#include "interpreter.h"

int eval(const char *src, Value *out);

