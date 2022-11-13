#include <string.h>

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

#define CHECK_CODE_SIZE(n)                         \
    if (vm->ip + n - 1 >= size)                    \
    {                                              \
        InterpretResult result;                    \
        result.code = INTERPRET_IP_OUT_OF_RANGE;   \
        result.detail.ip_out_of_range.ip = vm->ip; \
        return result;                             \
    }

#define READ_BYTE_INTO(v) \
    CHECK_CODE_SIZE(1);   \
    unsigned char v = code[vm->ip++];

#define READ_S32_INTO(v)                            \
    CHECK_CODE_SIZE(sizeof(int32_t))                \
    int32_t value;                                  \
    memcpy(&value, code + vm->ip, sizeof(int32_t)); \
    vm->ip += sizeof(int32_t);

#define PEEK(n) vm->stack[vm->sp - n - 1]

#define VALIDATE_STACK_UNDERFLOW(n)                \
    if (vm->sp < n)                                \
    {                                              \
        InterpretResult result;                    \
        result.code = INTERPRET_STACK_UNDERFLOW;   \
        result.detail.stack_underflow.ip = vm->ip; \
        return result;                             \
    }

#define VALIDATE_STACK_OVERFLOW(n)                \
    if (vm->sp + n - 1 == STACK_SIZE)             \
    {                                             \
        InterpretResult result;                   \
        result.code = INTERPRET_STACK_OVERFLOW;   \
        result.detail.stack_overflow.ip = vm->ip; \
        return result;                            \
    }

#define VALIDATE_ARGUMENT_TYPE(v, t)                                    \
    if (v.type != t)                                                    \
    {                                                                   \
        InterpretResult result;                                         \
        result.code = INTERPRET_INVALID_ARGUMENT_TYPES;                 \
        result.detail.invalid_argument_types.ip = vm->ip;               \
        result.detail.invalid_argument_types.instruction = instruction; \
        return result;                                                  \
    }

#define S32_OPERATOR(op)                                                                                     \
    VALIDATE_STACK_UNDERFLOW(2);                                                                             \
    VALIDATE_ARGUMENT_TYPE(PEEK(0), VT_S32);                                                                 \
    VALIDATE_ARGUMENT_TYPE(PEEK(1), VT_S32);                                                                 \
    vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 op vm->stack[vm->sp - 1].detail.s32; \
    vm->sp--;

InterpretResult bci_run(VM *vm)
{
    char *code = vm->chunk->code;
    const int32_t size = vm->chunk->size;

    for (;;)
    {
        READ_BYTE_INTO(instruction);

        switch (instruction)
        {
        case OP_ADD_S32:
        {
            S32_OPERATOR(+);
            break;
        }
        case OP_SUB_S32:
        {
            S32_OPERATOR(-);
            break;
        }
        case OP_MUL_S32:
        {
            S32_OPERATOR(*);
            break;
        }
        case OP_DIV_S32:
        {
            VALIDATE_STACK_UNDERFLOW(2);
            VALIDATE_ARGUMENT_TYPE(PEEK(0), VT_S32);
            VALIDATE_ARGUMENT_TYPE(PEEK(1), VT_S32);
            if (vm->stack[vm->sp - 1].detail.s32 == 0)
            {
                InterpretResult result;
                result.code = INTERPRET_DIVISION_BY_ZERO;
                result.detail.division_by_zero.ip = vm->ip;
                return result;
            }

            vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 / vm->stack[vm->sp - 1].detail.s32;
            vm->sp--;
            break;
        }
        case OP_PUSH_TRUE:
        {
            VALIDATE_STACK_OVERFLOW(1);

            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;

            break;
        }
        case OP_PUSH_FALSE:
        {
            VALIDATE_STACK_OVERFLOW(1);

            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;

            break;
        }
        case OP_PUSH_S32:
        {
            VALIDATE_STACK_OVERFLOW(1);
            READ_S32_INTO(value);

            vm->stack[vm->sp].type = VT_S32;
            vm->stack[vm->sp++].detail.s32 = value;

            break;
        }
        case OP_RET:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = vm->sp == 0 || vm->stack[vm->sp - 1].type != VT_S32 ? 0 : vm->stack[vm->sp - 1].detail.s32;

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
