#include "op.h"

char *Op_Names[] = {
    "RET",
    "RET_S32",
    "ADD_S32",
    "DIV_S32",
    "MUL_S32",
    "SUB_S32",
    "PUSH_FALSE",
    "PUSH_TRUE",
    "PUSH_S32"};

char *Op_name(Op op)
{
    if (op < 0 || op > 8)
    {
        return "UNKNOWN";
    }

    return Op_Names[op];
}
