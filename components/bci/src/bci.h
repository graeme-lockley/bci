#ifndef BCI_H
#define BCI_H

#include "chunk.h"

#define STACK_SIZE 256

typedef enum
{
    INTERPRET_OK,
    INTERPRET_IP_OUT_OF_RANGE,
    INTERPRET_INVALID_INSTRUCTION,
    INTERPRET_STACK_OVERFLOW,
    INTERPRET_STACK_UNDERFLOW,
    INTERPRET_INVALID_ARGUMENT_TYPES,
    INTERPRET_DIVISION_BY_ZERO
} InterpretResultCode;

typedef struct
{
    int32_t result;
} OKResult;

typedef struct
{
    int32_t ip;
} IPOutOfRangeError;

typedef struct
{
    int32_t ip;
    unsigned char instruction;
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

typedef struct {
    int32_t ip;
} DivisionByZeroError;

typedef union
{
    OKResult ok;
    IPOutOfRangeError ip_out_of_range;
    InvalidInstructionError invalid_instruction;
    StackOverflowError stack_overflow;
    StackUnderflowError stack_underflow;
    InvalidArgumentTypesError invalid_argument_types;
    DivisionByZeroError division_by_zero;
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

typedef struct
{
    Chunk *chunk;
    int32_t ip;
    Value stack[STACK_SIZE];
    int32_t sp;
} VM;

extern VM *bci_initVM_populate(Chunk *chunk);
extern void bci_freeVM(VM *vm);

extern InterpretResult bci_run(VM *vm);

#endif
