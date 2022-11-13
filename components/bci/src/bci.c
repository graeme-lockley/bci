#include "chunk.h"
#include "memory.h"
#include "op.h"

#include "bci.h"

VM *bci_initVM_populate(Chunk *chunk)
{
    VM *vm = ALLOCATE(VM, 1);

    vm->chunk = chunk;
    vm->ip = 0;

    return vm;
}

void bci_freeVM(VM *vm)
{
    chunk_free(vm->chunk);
    FREE(vm);
}

#define READ_BYTE_INTO(v)                          \
    if (vm->ip >= size)                            \
    {                                              \
        InterpretResult result;                    \
        result.code = INTERPRET_IP_OUT_OF_RANGE;   \
        result.detail.ip_out_of_range.ip = vm->ip; \
        return result;                             \
    }                                              \
    unsigned char v = code[vm->ip++];

InterpretResult bci_run(VM *vm)
{
    char *code = vm->chunk->code;
    const int size = vm->chunk->size;

    for (;;)
    {
        READ_BYTE_INTO(instruction);

        switch (instruction)
        {
        case OP_RET:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = 0;

            return result;
        }
        default:
        {
            InterpretResult result;

            result.code = INTERPRET_INVALID_INSTRUCTION;
            result.detail.invalid_instruction.ip = vm->ip;
            result.detail.invalid_instruction.instruction = instruction;

            return result;
        }
        }
    }
}

#undef READ_BYTE
