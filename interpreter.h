#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parse.h"

Value interpret(CalcCtx *ctx, Node *node);
void  public_set_var(const char *name, int len, Value val);

#endif
