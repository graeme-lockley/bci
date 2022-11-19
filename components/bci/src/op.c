#include "op.h"

char *Op_Names[] = {
    "ADD_S32",
    "DIV_S32",
    "MUL_S32",
    "SUB_S32",
    "PUSH_FALSE",
    "PUSH_TRUE",
    "PUSH_S32",
    "RET"};

char *Op_name(Op op)
{
    return Op_Names[op];
}
