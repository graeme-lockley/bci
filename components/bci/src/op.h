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
    EOP_PUSH_S32,

    EOP_JMP,
    EOP_BLOCK
} EOp;

typedef enum
{
    IOP_RET,
    IOP_RET_BOOL,
    IOP_RET_S32,

    IOP_ADD_S32,
    IOP_DIV_S32,
    IOP_MUL_S32,
    IOP_SUB_S32,

    IOP_PUSH_FALSE,
    IOP_PUSH_TRUE,
    IOP_PUSH_S32,

    IOP_JMP
} IOp;

extern char *EOp_Names[];
extern char *EOp_name(EOp eop);

extern char *IOp_Names[];
extern char *IOp_name(IOp iop);

#endif
