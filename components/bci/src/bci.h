#ifndef BCI_H
#define BCI_H

#include "block.h"

#define STACK_SIZE 256
#define BCI_VERBOSE

typedef enum
{
    INIT_OK,
    INIT_IP_OUT_OF_RANGE,
    INIT_INVALID_INSTRUCTION,
    INIT_STACK_OVERFLOW,
    INIT_STACK_UNDERFLOW,
    INIT_INVALID_ARGUMENT_TYPES,
    INIT_BLOCK_INCORRECTLY_TERMINATED,
    INIT_RET_MUST_TERMINATE_BLOCK,
    INIT_RET_INVALID_STACK,
} InitResultCode;

typedef struct
{
    struct VM *vm;
} OKInit;

typedef struct
{
    int32_t ip;
} IPOutOfRangeError;

typedef struct
{
    int32_t ip;
    unsigned int instruction;
} InvalidInstructionError;

typedef struct
{
    int32_t ip;
} StackOverflowError;

typedef struct
{
    int32_t ip;
} StackUnderflowError;

typedef struct
{
    int32_t ip;
    unsigned char instruction;
} InvalidArgumentTypesError;

typedef struct
{
    int32_t ip;
} BlockIncorrectlyTerminatedError;

typedef struct
{
    int32_t ip;
} RETMustTerminateBlockError;

typedef struct
{
    int32_t ip;
} RETInvalidStackError;

typedef union
{
    OKInit ok;
    IPOutOfRangeError ip_out_of_range;
    InvalidInstructionError invalid_instruction;
    StackOverflowError stack_overflow;
    StackUnderflowError stack_underflow;
    InvalidArgumentTypesError invalid_argument_types;
    BlockIncorrectlyTerminatedError block_incorrectly_terminated;
    RETMustTerminateBlockError ret_must_terminate_block;
    RETInvalidStackError ret_invalid_stack;
} InitResultDetail;

typedef struct
{
    InitResultCode code;
    InitResultDetail detail;
} InitResult;


typedef enum
{
    INTERPRET_OK,
    INTERPRET_INVALID_INSTRUCTION,
    INTERPRET_DIVISION_BY_ZERO
} InterpretResultCode;

typedef struct
{
    int32_t result;
} OKResult;

typedef struct
{
    int32_t ip;
} DivisionByZeroError;

typedef union
{
    OKResult ok;
    InvalidInstructionError invalid_instruction;
    DivisionByZeroError division_by_zero;
    BlockIncorrectlyTerminatedError block_incorrectly_terminated;
} ResultDetail;

typedef struct
{
    InterpretResultCode code;
    ResultDetail detail;
} InterpretResult;

typedef enum
{
    VT_S32,
    VT_BOOL
} ValueType;

typedef union
{
    int32_t s32;
} ValueContent;

typedef struct
{
    ValueType type;
    ValueContent detail;
} Value;

typedef struct VM
{
    Block *block;
    int32_t ip;
    Value stack[STACK_SIZE];
    int32_t sp;
} VM;

extern InitResult bci_initVM_populate(Block *block);
extern void bci_freeVM(VM *vm);

extern InterpretResult bci_run(VM *vm);

extern char *bci_initResult_toString(InitResult result);
extern char *bci_interpretResult_toString(InterpretResult result);

#endif
