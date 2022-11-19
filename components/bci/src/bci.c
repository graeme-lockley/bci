#include <stdio.h>
#include <string.h>

#include "block.h"
#include "memory.h"
#include "op.h"

#include "bci.h"

VM *bci_initVM_populate(Block *block)
{
    VM *vm = ALLOCATE(VM, 1);

    vm->block = block;
    vm->ip = 0;
    vm->sp = 0;

    return vm;
}

void bci_freeVM(VM *vm)
{
    block_free(vm->block);
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
    char *code = vm->block->code;
    const int32_t size = vm->block->size;

    int32_t ip = 0;
    int32_t sp = 0;
    ValueType stack[STACK_SIZE];

    for (;;)
    {
        if (ip >= vm->block->size)
        {
            InterpretResult result;

            result.code = INTERPRET_BLOCK_INCORRECTLY_TERMINATED;
            result.detail.block_incorrectly_terminated.ip = ip;

            return result;
        }
        Op op = vm->block->code[ip];
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
            if (ip + sizeof(int32_t) >= vm->block->size)
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
            if (ip != vm->block->size)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 0)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else
            {
                InterpretResult result;

                result.code = INTERPRET_OK;

                return result;
            }
        case OP_RET_BOOL:
            if (ip != vm->block->size)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 1)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (stack[0] != VT_BOOL)
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
        case OP_RET_S32:
            if (ip != vm->block->size)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 1)
            {
                InterpretResult result;

                result.code = INTERPRET_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (stack[0] != VT_S32)
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

#ifdef BCI_VERBOSE
    printf("bci_run: verification result: %d\n", result.code);
#endif

    if (result.code != INTERPRET_OK)
    {
        return result;
    }

    char *code = vm->block->code;
    const int32_t size = vm->block->size;

    for (;;)
    {
#ifdef BCI_VERBOSE
        printf("bci_run: ip=%04x", vm->ip);
#endif
        READ_BYTE_INTO(instruction);

#ifdef BCI_VERBOSE
        printf(", sp=%04x: %s (%d)\n", vm->sp, Op_name(instruction), instruction);
#endif

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
            vm->stack[vm->sp].detail.s32 = 0;
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
            result.detail.ok.result = 0;

            return result;
        }
        case OP_RET_BOOL:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = vm->stack[vm->sp - 1].detail.s32;

            return result;
        }
        case OP_RET_S32:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = vm->stack[vm->sp - 1].detail.s32;

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

char *bci_interpretResult_toString(InterpretResult result)
{
    char line[256];

    switch (result.code)
    {
    case INTERPRET_OK:
        sprintf(line, "OK, result: %d", result.detail.ok.result);
        break;
    case INTERPRET_INVALID_INSTRUCTION:
        sprintf(line, "Invalid instruction %s at %04x", Op_name(result.detail.invalid_instruction.instruction), result.detail.invalid_instruction.ip);
        break;
    case INTERPRET_INVALID_ARGUMENT_TYPES:
        sprintf(line, "Invalid argument types for %s at %04x", Op_name(result.detail.invalid_argument_types.instruction), result.detail.invalid_argument_types.ip);
        break;
    case INTERPRET_STACK_OVERFLOW:
        sprintf(line, "Stack overflow at %04x", result.detail.stack_overflow.ip);
        break;
    case INTERPRET_STACK_UNDERFLOW:
        sprintf(line, "Stack underflow at %04x", result.detail.stack_underflow.ip);
        break;
    case INTERPRET_BLOCK_INCORRECTLY_TERMINATED:
        sprintf(line, "Block incorrectly terminated at %04x", result.detail.block_incorrectly_terminated.ip);
        break;
    case INTERPRET_RET_MUST_TERMINATE_BLOCK:
        sprintf(line, "Ret must terminate block at %04x", result.detail.ret_must_terminate_block.ip);
        break;
    case INTERPRET_RET_INVALID_STACK:
        sprintf(line, "Ret invalid stack at %04x", result.detail.ret_invalid_stack.ip);
        break;
    case INTERPRET_DIVISION_BY_ZERO:
        sprintf(line, "Division by zero at %04x", result.detail.division_by_zero.ip);
        break;
    default:
        sprintf(line, "Unknown error %04x", result.code);
    }

    return strdup(line);
}
