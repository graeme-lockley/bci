#include <stdio.h>
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

#define S32_OPERATOR(op)                                                                                     \
    vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 op vm->stack[vm->sp - 1].detail.s32; \
    vm->sp--;

static InterpretResult verifyBlock(VM *vm)
{
    char *code = vm->chunk->code;
    const int32_t size = vm->chunk->size;

    int32_t ip = 0;
    int32_t sp = 0;
    ValueType stack[STACK_SIZE];

    for (;;)
    {
        if (ip >= vm->chunk->size)
        {
            InterpretResult result;

            result.code = INTERPRET_BLOCK_INCORRECTLY_TERMINATED;
            result.detail.block_incorrectly_terminated.ip = ip;

            return result;
        }
        Op op = vm->chunk->code[ip];
        ip += 1;
        switch (op)
        {
        case OP_PUSH_TRUE:
        case OP_PUSH_FALSE:
            if (sp == STACK_SIZE)
            {
                InterpretResult result;

                result.code = INTERPRET_STACK_OVERFLOW;
                result.detail.stack_overflow.ip = ip;

                return result;
            }
            stack[sp] = VT_BOOL;
            sp += 1;
            break;
        case OP_PUSH_S32:
            if (sp == STACK_SIZE)
            {
                InterpretResult result;

                result.code = INTERPRET_STACK_OVERFLOW;
                result.detail.stack_overflow.ip = ip;

                return result;
            }
            if (ip + sizeof(int32_t) >= vm->chunk->size)
            {
                InterpretResult result;

                result.code = INTERPRET_BLOCK_INCORRECTLY_TERMINATED;
                result.detail.block_incorrectly_terminated.ip = ip;

                return result;
            }
            stack[sp] = VT_S32;
            sp += 1;
            ip += sizeof(int32_t);
            break;
        case OP_ADD_S32:
        case OP_SUB_S32:
        case OP_MUL_S32:
        case OP_DIV_S32:
            if (sp < 2)
            {
                InterpretResult result;

                result.code = INTERPRET_STACK_UNDERFLOW;
                result.detail.stack_underflow.ip = ip;

                return result;
            }
            else if (stack[sp - 1] != VT_S32 || stack[sp - 2] != VT_S32)
            {
                InterpretResult result;

                result.code = INTERPRET_INVALID_ARGUMENT_TYPES;
                result.detail.invalid_argument_types.ip = ip;
                result.detail.invalid_argument_types.instruction = op;

                return result;
            }
            sp -= 1;
            break;
        case OP_RET:
            if (ip != vm->chunk->size)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp > 1)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp == 1 && stack[0] != VT_S32)
            {
                InterpretResult result;

                result.code = INTERPRET_INVALID_ARGUMENT_TYPES;
                result.detail.invalid_argument_types.ip = ip;
                result.detail.invalid_argument_types.instruction = op;

                return result;
            }
            else
            {
                InterpretResult result;

                result.code = INTERPRET_OK;

                return result;
            }
        default:
        {
            InterpretResult result;

            result.code = INTERPRET_INVALID_INSTRUCTION;
            result.detail.invalid_instruction.ip = ip;
            result.detail.invalid_instruction.instruction = op;

            return result;
        }
        }
    }
}

InterpretResult bci_run(VM *vm)
{
    InterpretResult result = verifyBlock(vm);
    if (result.code != INTERPRET_OK)
    {
        return result;
    }

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
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;

            break;
        }
        case OP_PUSH_FALSE:
        {
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;

            break;
        }
        case OP_PUSH_S32:
        {
            READ_S32_INTO(value);

            vm->stack[vm->sp].type = VT_S32;
            vm->stack[vm->sp].detail.s32 = value;
            vm->sp++;

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
