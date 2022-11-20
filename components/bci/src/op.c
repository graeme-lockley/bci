#include "op.h"

char *EOp_Names[] = {
    "RET",
    "RET_BOOL",
    "RET_S32",
    "ADD_S32",
    "DIV_S32",
    "MUL_S32",
    "SUB_S32",
    "PUSH_FALSE",
    "PUSH_TRUE",
    "PUSH_S32",
    "BLOCK",
    "JMP"};

char *EOp_name(EOp eop)
{
    if (eop < 0 || eop > 11)
    {
        return "UNKNOWN";
    }

    return EOp_Names[eop];
}

char *IOp_Names[] = {
    "RET",
    "RET_BOOL",
    "RET_S32",
    "ADD_S32",
    "DIV_S32",
    "MUL_S32",
    "SUB_S32",
    "PUSH_FALSE",
    "PUSH_TRUE",
    "PUSH_S32",
    "JMP"};

char *IOp_name(IOp iop)
{
    if (iop < 0 || iop > 10)
    {
        return "UNKNOWN";
    }

    return IOp_Names[iop];
}
