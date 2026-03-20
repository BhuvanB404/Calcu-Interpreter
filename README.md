# Calcu-Interpreter
A simple calculator that does  addition, substraction, multiplication, division. 
It handles precidence and proper parsing of input stream.
It contains a lexer,parser, an AST and interpreter for traversing and solving the AST
Build using ```make``` 

# The Deisgn Choices 

## Stage 1 -> the input handling 

The input handling is a REPL style loop, 

- Uses fgets to get terminal input. 
- Tracks history of operations upto last 3 operations 
- Can do multi line input using ';' 
- can type ```ANS``` to get last operation ans 


## Stage 2 -> The Lexical Analysis

- It uses a token table to keep track of the tokens that are valid
- Does not use null temrinated strings, but the raw strings, to eliminate string duplication  of input string for evaluation of nested functions 
- Has a Token struct with position ,type and heap location info. 
 ```C
        struct Token {
  Tokentype   type;   // Token category
  Token      *next;   // Next token in list
  const char *loc;    // Points into source string
  int         len;    // Length in bytes
};
```
- Tokens are stoed in a linked list. 


## Stage 3 -> Parsing 

For parsing, The design choice has deviated from the normal choice of recursive descent parsing, instead *Pratt parser* is used for its superior expression handling .

- AST representation is through Node types which have heirarchial state logic  similar to Tokens but more verbose
- The parser uses binding power as a substitute for precidence check . 
- The left and right tokens are given power values based on the operator in between them . 
  ```Assignment =: l_bp=2, r_bp=1 (right-associative)
    Add/Sub +/-: l_bp=3, r_bp=4 (left-associative)
    Mul/Div * / /: l_bp=5, r_bp=6 (left-associative)
    Power **: l_bp=8, r_bp=7 (right-associative) 
- the relative values of l_bp and r_bp amongst same operations  provide the associativelty, 
- the relative values of l_bp and r_bp among different operations provide the precidence order. 
```+ = r_bp  = 4; 
    * = l_bp  = 5;
    5> 4 , hence multiplcation has higher order of importance
```
Example : 
```C
Input: "x = 2 + 3"

parse():
  init_rules()
  parse_expr(tok=TK_IDENT("x"), min_bp=0):
    prefix_ident() → lhs = NODE_VAR("x")
    see TK_ASSIGN (l_bp=2 > 0)
    infix_op(=):
      validate lhs is NODE_VAR
      parse_expr(tok=TK_INT("2"), min_bp=1):
        prefix_int() → lhs = NODE_LITERAL(2)
        see TK_PLUS (l_bp=3 > 1)
        infix_op(+):
          parse_expr(tok=TK_INT("3"), min_bp=4):
            prefix_int() → NODE_LITERAL(3)
            EOF → return
          return NODE_BINARY(+, 2, 3)
      return NODE_ASSIGN(x, NODE_BINARY(+, 2, 3))
```


## Stage 4 -> Evaluation

For evaluation from the AST, a tree walker is deployed.

- Due to int and float values, a union is used for runtime selection 

```C
typedef struct {
  ValType type;      // VAL_INT or VAL_FLOAT
  union {
    long   ival;     // Integer value
    double fval;     // Float value
  } as;
} Value;
```
- The tree walker recursively walks the AST tree, evaluating the children.
- In case of mixed datatype evalutaion, they are promoted to higher order 
- A table is used for mapping the differnt functions to their nodetypes 
- each node type has its own function 
- for function calls such as ```sin(1)``` , ```eval_call() ```if used which does function validation, lookup , argument evaluaition and calls respective function.
- functions are mapped in this way through the use of table + functional relation: 
```C
bin_int_table[OP_ADD] → bin_add_int()
bin_float_table[OP_ADD] → bin_add_float()
``` 
- the trignometrical functions provied take radian as input 

example : 
```C
AST: NODE_BINARY(+, NODE_LITERAL(2), NODE_BINARY(*, NODE_LITERAL(3), NODE_LITERAL(4)))

eval_node(NODE_BINARY(+)):
  eval_binary():
    lhs = eval_node(NODE_LITERAL(2)):
            eval_literal() → Value{VAL_INT, 2}
    
    rhs = eval_node(NODE_BINARY(*)):
            eval_binary():
              lhs = eval_node(NODE_LITERAL(3)):
                      eval_literal() → Value{VAL_INT, 3}
              rhs = eval_node(NODE_LITERAL(4)):
                      eval_literal() → Value{VAL_INT, 4}
              promote(&lhs, &rhs) → no change (both int)
              op = OP_MUL
              f = bin_int_table[OP_MUL] = bin_mul_int
              return f(3, 4) → Value{VAL_INT, 12}
    
    promote(&lhs, &rhs) → no change (both int)
    op = OP_ADD
    f = bin_int_table[OP_ADD] = bin_add_int
    return f(2, 12) → Value{VAL_INT, 14}
```

## Stage 5 -> Error handling 

- Error hadnling is done Globally  thorugh ```error.c``` 
- Using a Ctx struct and setjmp function, snapshot error handling is implemented. Reverting to stable state when an error is encountered.


 
