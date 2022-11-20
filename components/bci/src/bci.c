#include <stdio.h>
#include <string.h>

#include "block.h"
#include "memory.h"
#include "op.h"

#include "bci.h"

static InitResult verifyBlock(Block *block)
{
    char *code = block->code;
    const int32_t size = block->size;

    int32_t ip = 0;
    int32_t sp = 0;
    ValueType stack[STACK_SIZE];

    for (;;)
    {
        if (ip >= size)
        {
            InitResult result;

            result.code = INIT_BLOCK_INCORRECTLY_TERMINATED;
            result.detail.block_incorrectly_terminated.ip = ip;

            return result;
        }
        EOp op = code[ip];
        ip += 1;
        switch (op)
        {
        case EOP_PUSH_TRUE:
        case EOP_PUSH_FALSE:
            if (sp == STACK_SIZE)
            {
                InitResult result;

                result.code = INIT_STACK_OVERFLOW;
                result.detail.stack_overflow.ip = ip;

                return result;
            }
            stack[sp] = VT_BOOL;
            sp += 1;
            break;
        case EOP_PUSH_S32:
            if (sp == STACK_SIZE)
            {
                InitResult result;

                result.code = INIT_STACK_OVERFLOW;
                result.detail.stack_overflow.ip = ip;

                return result;
            }
            if (ip + sizeof(int32_t) >= size)
            {
                InitResult result;

                result.code = INIT_BLOCK_INCORRECTLY_TERMINATED;
                result.detail.block_incorrectly_terminated.ip = ip;

                return result;
            }
            stack[sp] = VT_S32;
            sp += 1;
            ip += sizeof(int32_t);
            break;
        case EOP_ADD_S32:
        case EOP_SUB_S32:
        case EOP_MUL_S32:
        case EOP_DIV_S32:
            if (sp < 2)
            {
                InitResult result;

                result.code = INIT_STACK_UNDERFLOW;
                result.detail.stack_underflow.ip = ip;

                return result;
            }
            else if (stack[sp - 1] != VT_S32 || stack[sp - 2] != VT_S32)
            {
                InitResult result;

                result.code = INIT_INVALID_ARGUMENT_TYPES;
                result.detail.invalid_argument_types.ip = ip;
                result.detail.invalid_argument_types.instruction = op;

                return result;
            }
            sp -= 1;
            break;
        case EOP_RET:
            if (ip != size)
            {
                InitResult result;

                result.code = INIT_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 0)
            {
                InitResult result;

                result.code = INIT_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else
            {
                InitResult result;

                result.code = INIT_OK;
                result.detail.ok.vm = NULL;

                return result;
            }
        case EOP_RET_BOOL:
            if (ip != size)
            {
                InitResult result;

                result.code = INIT_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 1)
            {
                InitResult result;

                result.code = INIT_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (stack[0] != VT_BOOL)
            {
                InitResult result;

                result.code = INIT_INVALID_ARGUMENT_TYPES;
                result.detail.invalid_argument_types.ip = ip;
                result.detail.invalid_argument_types.instruction = op;

                return result;
            }
            else
            {
                InitResult result;

                result.code = INIT_OK;

                return result;
            }
        case EOP_RET_S32:
            if (ip != size)
            {
                InitResult result;

                result.code = INIT_RET_MUST_TERMINATE_BLOCK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (sp != 1)
            {
                InitResult result;

                result.code = INIT_RET_INVALID_STACK;
                result.detail.ret_must_terminate_block.ip = ip;

                return result;
            }
            else if (stack[0] != VT_S32)
            {
                InitResult result;

                result.code = INIT_INVALID_ARGUMENT_TYPES;
                result.detail.invalid_argument_types.ip = ip;
                result.detail.invalid_argument_types.instruction = op;

                return result;
            }
            else
            {
                InitResult result;

                result.code = INIT_OK;

                return result;
            }
        default:
        {
            InitResult result;

            result.code = INIT_INVALID_INSTRUCTION;
            result.detail.invalid_instruction.ip = ip;
            result.detail.invalid_instruction.instruction = op;

            return result;
        }
        }
    }
}

InitResult bci_initVM_populate(Block *block)
{
    InitResult result = verifyBlock(block);

    if (result.code == INIT_OK)
    {
        VM *vm = ALLOCATE(VM, 1);

        vm->block = block;
        vm->ip = 0;
        vm->sp = 0;

        result.detail.ok.vm = vm;
    } else {
        block_free(block);
    }

    return result;
}

void bci_freeVM(VM *vm)
{
    block_free(vm->block);
    FREE(vm);
}

#define READ_BYTE_INTO(v) \
    unsigned char v = code[vm->ip++];

#define READ_S32_INTO(v)                            \
    int32_t value;                                  \
    memcpy(&value, code + vm->ip, sizeof(int32_t)); \
    vm->ip += sizeof(int32_t);

#define S32_OPERATOR(op)                                                                                     \
    vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 op vm->stack[vm->sp - 1].detail.s32; \
    vm->sp--;

InterpretResult bci_run(VM *vm)
{
    vm->ip = 0;
    vm->sp = 0;

    char *code = vm->block->code;
    const int32_t size = vm->block->size;

    for (;;)
    {
#ifdef BCI_VERBOSE
        printf("bci_run: ip=%04x", vm->ip);
#endif
        READ_BYTE_INTO(instruction);

#ifdef BCI_VERBOSE
        printf(", sp=%04x: %s (%d)\n", vm->sp, EOp_name(instruction), instruction);
#endif

        switch (instruction)
        {
        case EOP_ADD_S32:
        {
            S32_OPERATOR(+);
            break;
        }
        case EOP_SUB_S32:
        {
            S32_OPERATOR(-);
            break;
        }
        case EOP_MUL_S32:
        {
            S32_OPERATOR(*);
            break;
        }
        case EOP_DIV_S32:
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
        case EOP_PUSH_TRUE:
        {
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;

            break;
        }
        case EOP_PUSH_FALSE:
        {
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 0;
            vm->sp++;

            break;
        }
        case EOP_PUSH_S32:
        {
            READ_S32_INTO(value);

            vm->stack[vm->sp].type = VT_S32;
            vm->stack[vm->sp].detail.s32 = value;
            vm->sp++;

            break;
        }
        case EOP_RET:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = 0;

            return result;
        }
        case EOP_RET_BOOL:
        {
            InterpretResult result;

            result.code = INTERPRET_OK;
            result.detail.ok.result = vm->stack[vm->sp - 1].detail.s32;

            return result;
        }
        case EOP_RET_S32:
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

char *bci_initResult_toString(InitResult result)
{
    char line[256];

    switch (result.code)
    {
    case INIT_OK:
        sprintf(line, "OK");
        break;
    case INIT_INVALID_INSTRUCTION:
        sprintf(line, "Invalid instruction %s at %04x", EOp_name(result.detail.invalid_instruction.instruction), result.detail.invalid_instruction.ip);
        break;
    case INIT_INVALID_ARGUMENT_TYPES:
        sprintf(line, "Invalid argument types for %s at %04x", EOp_name(result.detail.invalid_argument_types.instruction), result.detail.invalid_argument_types.ip);
        break;
    case INIT_STACK_OVERFLOW:
        sprintf(line, "Stack overflow at %04x", result.detail.stack_overflow.ip);
        break;
    case INIT_STACK_UNDERFLOW:
        sprintf(line, "Stack underflow at %04x", result.detail.stack_underflow.ip);
        break;
    case INIT_BLOCK_INCORRECTLY_TERMINATED:
        sprintf(line, "Block incorrectly terminated at %04x", result.detail.block_incorrectly_terminated.ip);
        break;
    case INIT_RET_MUST_TERMINATE_BLOCK:
        sprintf(line, "Ret must terminate block at %04x", result.detail.ret_must_terminate_block.ip);
        break;
    case INIT_RET_INVALID_STACK:
        sprintf(line, "Ret invalid stack at %04x", result.detail.ret_invalid_stack.ip);
        break;
    default:
        sprintf(line, "Unknown error %04x", result.code);
    }

    return strdup(line);
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
        sprintf(line, "Invalid instruction %s at %04x", EOp_name(result.detail.invalid_instruction.instruction), result.detail.invalid_instruction.ip);
        break;
    case INTERPRET_DIVISION_BY_ZERO:
        sprintf(line, "Division by zero at %04x", result.detail.division_by_zero.ip);
        break;
    default:
        sprintf(line, "Unknown error %04x", result.code);
    }

    return strdup(line);
}
