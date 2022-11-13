#ifndef BCI_H
#define BCI_H

#include "chunk.h"

typedef enum
{
    INTERPRET_OK,
    INTERPRET_IP_OUT_OF_RANGE,
    INTERPRET_INVALID_INSTRUCTION
} InterpretResultCode;

typedef struct
{
    int result;
} OKResult;

    typedef struct
{
    int ip;
} IPOutOfRangeError;

typedef struct
{
    int ip;
    unsigned char instruction;
} InvalidInstructionError;

typedef union
{
    OKResult ok;
    IPOutOfRangeError ip_out_of_range;
    InvalidInstructionError invalid_instruction;
} ResultDetail;

typedef struct
{
    InterpretResultCode code;
    ResultDetail detail;
} InterpretResult;

typedef struct
{
    Chunk *chunk;
    int ip;
} VM;

extern VM *bci_initVM_populate(Chunk *chunk);
extern void bci_freeVM(VM *vm);

extern InterpretResult bci_run(VM *vm);

#endif
