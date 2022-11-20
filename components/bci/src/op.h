#ifndef OP_H
#define OP_H

typedef enum
{
    EOP_RET,
    EOP_RET_BOOL,
    EOP_RET_S32,

    EOP_ADD_S32,
    EOP_DIV_S32,
    EOP_MUL_S32,
    EOP_SUB_S32,

    EOP_PUSH_FALSE,
    EOP_PUSH_TRUE,
    EOP_PUSH_S32
} EOp;

extern char *EOp_Names[];
extern char *EOp_name(EOp eop);

#endif
