#ifndef OP_H
#define OP_H

typedef enum
{
    OP_RET,
    OP_RET_BOOL,
    OP_RET_S32,

    OP_ADD_S32,
    OP_DIV_S32,
    OP_MUL_S32,
    OP_SUB_S32,

    OP_PUSH_FALSE,
    OP_PUSH_TRUE,
    OP_PUSH_S32
} Op;

extern char *Op_Names[];
extern char *Op_name(Op op);

#endif
