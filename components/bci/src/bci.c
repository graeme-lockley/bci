#include <stdio.h>
#include <string.h>

#include "block.h"
#include "buffer.h"
#include "map.h"
#include "memory.h"
#include "op.h"

#include "bci.h"

struct ReferencedLabels
{
    char *label;
    int32_t offset;
    struct ReferencedLabels *next;
};

typedef struct
{
    Buffer *buffer;
    struct ReferencedLabels *referenced_labels;
    Map *labels;

} IBlockBuilder;

IBlockBuilder *iblock_builder_new(void)
{
    IBlockBuilder *builder = ALLOCATE(IBlockBuilder, 1);
    builder->buffer = buffer_new(sizeof(char));
    builder->referenced_labels = NULL;
    builder->labels = map_string_new((void (*)(void *))NULL);

    return builder;
}

static void iblock_builder_appendIOP(IBlockBuilder *ibb, IOp iop)
{
    // buffer_append(buffer, &iop, sizeof(IOp));
    char c = (char)iop;
    buffer_append(ibb->buffer, &c, sizeof(char));
}

static void iblock_builder_appendIOPS32(IBlockBuilder *ibb, IOp iop, int32_t u32)
{
    iblock_builder_appendIOP(ibb, iop);
    buffer_append(ibb->buffer, &u32, sizeof(int32_t));
}

static void iblock_builder_appendIOPLabel(IBlockBuilder *ibb, IOp iop, char *label)
{
    iblock_builder_appendIOP(ibb, iop);
    int32_t offset = buffer_offset(ibb->buffer);
    int32_t u32 = 0;
    buffer_append(ibb->buffer, &u32, sizeof(int32_t));

    struct ReferencedLabels *rl = ALLOCATE(struct ReferencedLabels, 1);
    rl->label = STRDUP(label);
    rl->offset = offset;
    rl->next = ibb->referenced_labels;
    ibb->referenced_labels = rl;
}

static void iblock_builder_add_block(IBlockBuilder *ibb, char *name)
{
    if (map_get(ibb->labels, name) == NULL)
    {
        map_add(ibb->labels, STRDUP(name), (void *)((long)buffer_offset(ibb->buffer)));
    }
}

static void referenced_labels_free(struct ReferencedLabels *labels)
{
    while (labels)
    {
        struct ReferencedLabels *next = labels->next;
        FREE(labels->label);
        FREE(labels);
        labels = next;
    }
}

static void iblock_builder_patch_ip(IBlockBuilder *ibb, int32_t offset, int32_t ip)
{
    buffer_write(ibb->buffer, offset, &ip, sizeof(int32_t));
}

static char *iblock_builder_free_use(IBlockBuilder *ibb)
{
    char *result = buffer_free_use(ibb->buffer);
    referenced_labels_free(ibb->referenced_labels);
    map_free(ibb->labels);
    FREE(ibb);
    return result;
}

static void iblock_builder_free(IBlockBuilder *ibb)
{
    buffer_free(ibb->buffer);
    referenced_labels_free(ibb->referenced_labels);
    map_free(ibb->labels);
    FREE(ibb);
}

#define READ_STRING_INTO(v)      \
    char *v = (char *)code + ip; \
    ip += strlen(v) + 1;

static InitResult verifyTargetBlockStack(Blocks *blocks, ValueType *stack, int32_t sp, char *target_block_name)
{
    Block *target = map_get(blocks, target_block_name);

    if (target == NULL)
    {
        InitResult result;

        result.code = INIT_INVALID_LABEL;
        strcpy(result.detail.invalid_label.label, target_block_name);

        return result;
    }
    else if (target->types_size == -1)
    {
        target->types = ALLOCATE(ValueType, sp);
        target->types_size = sp;
        memcpy(target->types, stack, sp * sizeof(ValueType));
    }
    else if (target->types_size != sp || memcmp(target->types, stack, sp * sizeof(ValueType)) != 0)
    {
        InitResult result;

        result.code = INIT_BLOCK_STACKS_ARE_NOT_EQUAL;
        strcpy(result.detail.block_stacks_are_not_equal.label, target_block_name);

        return result;
    }

    return (InitResult){
        .code = INIT_OK,
        .detail.ok.vm = NULL};
}

static InitResult verifyBlock(Blocks *blocks, Block *block)
{
    char *code = block->code;
    const int32_t size = block->size;

    int32_t ip = 0;
    int32_t sp = block->types_size;
    ValueType stack[STACK_SIZE];

    memcpy(stack, block->types, block->types_size * sizeof(ValueType));

    for (;;)
    {
        if (ip >= size)
        {
            return (InitResult){
                .code = INIT_BLOCK_INCORRECTLY_TERMINATED,
                .detail.block_incorrectly_terminated.ip = ip};
        }
        EOp op = code[ip];
        ip += 1;
        switch (op)
        {
        case EOP_PUSH_TRUE:
        case EOP_PUSH_FALSE:
            if (sp == STACK_SIZE)
            {
                return (InitResult){
                    .code = INIT_STACK_OVERFLOW,
                    .detail.stack_overflow.ip = ip};
            }
            stack[sp] = VT_BOOL;
            sp += 1;
            break;
        case EOP_PUSH_S32:
            if (sp == STACK_SIZE)
            {
                return (InitResult){
                    .code = INIT_STACK_OVERFLOW,
                    .detail.stack_overflow.ip = ip};
            }
            if (ip + sizeof(int32_t) >= size)
            {
                return (InitResult){
                    .code = INIT_BLOCK_INCORRECTLY_TERMINATED,
                    .detail.block_incorrectly_terminated.ip = ip};
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
                return (InitResult){
                    .code = INIT_STACK_UNDERFLOW,
                    .detail.stack_underflow.ip = ip};
            }
            else if (stack[sp - 1] != VT_S32 || stack[sp - 2] != VT_S32)
            {
                return (InitResult){
                    .code = INIT_INVALID_ARGUMENT_TYPES,
                    .detail.invalid_argument_types.ip = ip,
                    .detail.invalid_argument_types.instruction = op};
            }
            sp -= 1;
            break;
        case EOP_RET:
            if (ip != size)
            {
                return (InitResult){
                    .code = INIT_RET_OR_JMP_MUST_TERMINATE_BLOCK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (sp != 0)
            {
                return (InitResult){
                    .code = INIT_RET_INVALID_STACK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else
            {
                return (InitResult){
                    .code = INIT_OK,
                    .detail.ok.vm = NULL};
            }
        case EOP_RET_BOOL:
            if (ip != size)
            {
                return (InitResult){
                    .code = INIT_RET_OR_JMP_MUST_TERMINATE_BLOCK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (sp != 1)
            {
                return (InitResult){
                    .code = INIT_RET_INVALID_STACK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (stack[0] != VT_BOOL)
            {
                return (InitResult){
                    .code = INIT_INVALID_ARGUMENT_TYPES,
                    .detail.invalid_argument_types.ip = ip,
                    .detail.invalid_argument_types.instruction = op};
            }
            else
            {
                return (InitResult){
                    .code = INIT_OK,
                    .detail.ok.vm = NULL};
            }
        case EOP_RET_S32:
            if (ip != size)
            {
                return (InitResult){
                    .code = INIT_RET_OR_JMP_MUST_TERMINATE_BLOCK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (sp != 1)
            {
                return (InitResult){
                    .code = INIT_RET_INVALID_STACK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (stack[0] != VT_S32)
            {
                return (InitResult){
                    .code = INIT_INVALID_ARGUMENT_TYPES,
                    .detail.invalid_argument_types.ip = ip,
                    .detail.invalid_argument_types.instruction = op};
            }
            else
            {
                return (InitResult){
                    .code = INIT_OK,
                    .detail.ok.vm = NULL};
            }
        case EOP_JMP:
        {
            READ_STRING_INTO(s);

            if (ip != size)
            {
                return (InitResult){
                    .code = INIT_RET_OR_JMP_MUST_TERMINATE_BLOCK,
                    .detail.ret_must_terminate_block.ip = ip};
            }

            return verifyTargetBlockStack(blocks, stack, sp, s);
        }
        case EOP_JMP_TRUE:
        {
            READ_STRING_INTO(s);

            if (sp == 0)
            {
                return (InitResult){
                    .code = INIT_RET_INVALID_STACK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (stack[sp - 1] != VT_BOOL)
            {
                return (InitResult){
                    .code = INIT_INVALID_ARGUMENT_TYPES,
                    .detail.invalid_argument_types.ip = ip,
                    .detail.invalid_argument_types.instruction = op};
            }

            sp -= 1;

            return verifyTargetBlockStack(blocks, stack, sp, s);
        }
        case EOP_JMP_FALSE:
        {
            READ_STRING_INTO(s);

            if (sp == 0)
            {
                return (InitResult){
                    .code = INIT_RET_INVALID_STACK,
                    .detail.ret_must_terminate_block.ip = ip};
            }
            else if (stack[sp - 1] != VT_BOOL)
            {
                return (InitResult){
                    .code = INIT_INVALID_ARGUMENT_TYPES,
                    .detail.invalid_argument_types.ip = ip,
                    .detail.invalid_argument_types.instruction = op};
            }

            sp -= 1;

            return verifyTargetBlockStack(blocks, stack, sp, s);
        }
        default:
            return (InitResult){
                .code = INIT_INVALID_INSTRUCTION,
                .detail.invalid_instruction.ip = ip,
                .detail.invalid_instruction.instruction = op};
        }
    }
}
#undef READ_STRING_INTO

#define READ_STRING_INTO(v)             \
    char *v = (char *)block->code + ip; \
    ip += strlen(v) + 1;

static InitResult compileBlock(Block *block, IBlockBuilder *ibb)
{
    int32_t ip = 0;

    while (ip < block->size)
    {
        EOp op = block->code[ip];
        ip += 1;
        switch (op)
        {
        case EOP_PUSH_TRUE:
            iblock_builder_appendIOP(ibb, IOP_PUSH_TRUE);
            break;
        case EOP_PUSH_FALSE:
            iblock_builder_appendIOP(ibb, IOP_PUSH_FALSE);
            break;
        case EOP_PUSH_S32:
            iblock_builder_appendIOPS32(ibb, IOP_PUSH_S32, *(int32_t *)(block->code + ip));
            ip += sizeof(int32_t);
            break;
        case EOP_ADD_S32:
            iblock_builder_appendIOP(ibb, IOP_ADD_S32);
            break;
        case EOP_SUB_S32:
            iblock_builder_appendIOP(ibb, IOP_SUB_S32);
            break;
        case EOP_MUL_S32:
            iblock_builder_appendIOP(ibb, IOP_MUL_S32);
            break;
        case EOP_DIV_S32:
            iblock_builder_appendIOP(ibb, IOP_DIV_S32);
            break;
        case EOP_RET:
            iblock_builder_appendIOP(ibb, IOP_RET);
            break;
        case EOP_RET_BOOL:
            iblock_builder_appendIOP(ibb, IOP_RET_BOOL);
            break;
        case EOP_RET_S32:
            iblock_builder_appendIOP(ibb, IOP_RET_S32);
            break;
        case EOP_JMP:
        {
            READ_STRING_INTO(s);
            iblock_builder_appendIOPLabel(ibb, IOP_JMP, s);
            break;
        }
        case EOP_JMP_TRUE:
        {
            READ_STRING_INTO(s);
            iblock_builder_appendIOPLabel(ibb, IOP_JMP_TRUE, s);
            break;
        }
        case EOP_JMP_FALSE:
        {
            READ_STRING_INTO(s);
            iblock_builder_appendIOPLabel(ibb, IOP_JMP_FALSE, s);
            break;
        }
        case EOP_BLOCK:
        {
            READ_STRING_INTO(s);
            ip += strlen(s) + 1;

            break;
        }
        default:
            return (InitResult){
                .code = INIT_INVALID_INSTRUCTION,
                .detail.invalid_instruction.ip = ip,
                .detail.invalid_instruction.instruction = op};
        }
    }

    return (InitResult){
        .code = INIT_OK,
        .detail.ok.vm = NULL};
}
#undef READ_STRING_INTO

static void initialiseInitialBlockStack(Block *initialBlock)
{
    initialBlock->types = ALLOCATE(ValueType, 1);
    initialBlock->types_size = 0;
}

static InitResult validateIndividualBlock(Blocks *blocks, Map_Node *node, int *block_updated)
{
    InitResult result = (InitResult){
        .code = INIT_OK,
        .detail.ok.vm = NULL};

    if (node)
    {
        Block *block = (Block *)node->value;

        if (!block->verified && block->types_size >= 0)
        {
            result = verifyBlock(blocks, block);

            block->verified = 1;
        }

        if (result.code == INIT_OK)
        {
            result = validateIndividualBlock(blocks, node->left, block_updated);
        }
        if (result.code == INIT_OK)
        {
            result = validateIndividualBlock(blocks, node->right, block_updated);
        }
    }

    return result;
}

static InitResult validateBlocks(Blocks *blocks)
{
    initialiseInitialBlockStack(blocks->root->value);

    while (1)
    {
        int block_updated = 0;

        InitResult result = validateIndividualBlock(blocks, blocks->root, &block_updated);

        if (result.code != INIT_OK)
        {
            return result;
        }

        if (!block_updated)
        {
            return (InitResult){
                .code = INIT_OK,
                .detail.ok.vm = NULL};
        }
    }
}

static InitResult populateBlock(IBlockBuilder *ibb, Map_Node *node)
{
    InitResult result = (InitResult){
        .code = INIT_OK,
        .detail.ok.vm = NULL};

    if (node)
    {
        Block *block = (Block *)node->value;
        iblock_builder_add_block(ibb, node->key);

        result = compileBlock(block, ibb);

        if (result.code == INIT_OK)
        {
            result = populateBlock(ibb, node->left);
        }

        if (result.code == INIT_OK)
        {
            result = populateBlock(ibb, node->right);
        }
    }

    return result;
}

static InitResult patchBlocks(IBlockBuilder *ibb)
{
    struct ReferencedLabels *patch = ibb->referenced_labels;

    while (patch)
    {
        int32_t ip = (int32_t)((long)map_get(ibb->labels, patch->label));

        if (ip == 0)
        {
            InitResult result;

            result.code = INIT_INVALID_LABEL;
            strcpy(result.detail.invalid_label.label, patch->label);

            return result;
        }

        iblock_builder_patch_ip(ibb, patch->offset, ip);

        patch = patch->next;
    }

    return (InitResult){
        .code = INIT_OK,
        .detail.ok.vm = NULL};
}

InitResult bci_initVM_populate(Blocks *blocks)
{
    InitResult result = validateBlocks(blocks);
    if (result.code != INIT_OK)
    {
        blocks_free(blocks);
        return result;
    }

    IBlockBuilder *ibb = iblock_builder_new();

    result = populateBlock(ibb, blocks->root);

    if (result.code == INIT_OK)
    {
        result = patchBlocks(ibb);
    }

    if (result.code == INIT_OK)
    {
        VM *vm = ALLOCATE(VM, 1);

        vm->code = iblock_builder_free_use(ibb);
        vm->ip = 0;
        vm->sp = 0;

        result.detail.ok.vm = vm;
    }
    else
    {
        iblock_builder_free(ibb);
    }

    blocks_free(blocks);

    return result;
}

void bci_freeVM(VM *vm)
{
    FREE(vm->code);
    FREE(vm);
}

#define READ_BYTE_INTO(v) \
    unsigned char v = code[vm->ip++];

#define READ_S32_INTO(v)                       \
    int32_t v = *((int32_t *)(code + vm->ip)); \
    vm->ip += sizeof(int32_t);

InterpretResult bci_run(VM *vm)
{
    vm->ip = 0;
    vm->sp = 0;

    char *code = vm->code;

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
        case IOP_ADD_S32:
            vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 + vm->stack[vm->sp - 1].detail.s32;
            vm->sp--;
            break;

        case IOP_SUB_S32:
            vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 - vm->stack[vm->sp - 1].detail.s32;
            vm->sp--;
            break;

        case IOP_MUL_S32:
            vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 * vm->stack[vm->sp - 1].detail.s32;
            vm->sp--;
            break;

        case IOP_DIV_S32:
            if (vm->stack[vm->sp - 1].detail.s32 == 0)
            {
                return (InterpretResult){
                    .code = INTERPRET_DIVISION_BY_ZERO,
                    .detail.division_by_zero.ip = vm->ip};
            }

            vm->stack[vm->sp - 2].detail.s32 = vm->stack[vm->sp - 2].detail.s32 / vm->stack[vm->sp - 1].detail.s32;
            vm->sp--;
            break;

        case IOP_PUSH_TRUE:
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 1;
            vm->sp++;
            break;

        case IOP_PUSH_FALSE:
            vm->stack[vm->sp].type = VT_BOOL;
            vm->stack[vm->sp].detail.s32 = 0;
            vm->sp++;
            break;

        case IOP_PUSH_S32:
        {
            READ_S32_INTO(value);

            vm->stack[vm->sp].type = VT_S32;
            vm->stack[vm->sp].detail.s32 = value;
            vm->sp++;

            break;
        }
        case IOP_RET:
            return (InterpretResult){
                .code = INTERPRET_OK,
                .detail.ok.result = 0};

        case IOP_RET_BOOL:
            return (InterpretResult){
                .code = INTERPRET_OK,
                .detail.ok.result = vm->stack[vm->sp - 1].detail.s32};

        case IOP_RET_S32:
            return (InterpretResult){
                .code = INTERPRET_OK,
                .detail.ok.result = vm->stack[vm->sp - 1].detail.s32};

        case IOP_JMP:
        {
            READ_S32_INTO(offset);
            vm->ip = offset;
            break;
        }
        case IOP_JMP_TRUE:
        {
            READ_S32_INTO(offset);
            if (vm->stack[vm->sp - 1].detail.s32 == 1)
            {
                vm->ip = offset;
            }
            vm->sp -= 1;
            break;
        }
        case IOP_JMP_FALSE:
        {
            READ_S32_INTO(offset);
            if (vm->stack[vm->sp - 1].detail.s32 == 0)
            {
                vm->ip = offset;
            }
            vm->sp -= 1;
            break;
        }
        default:
            return (InterpretResult){
                .code = INTERPRET_INVALID_INSTRUCTION,
                .detail.invalid_instruction.ip = vm->ip,
                .detail.invalid_instruction.instruction = instruction};
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
    case INIT_BLOCK_STACKS_ARE_NOT_EQUAL:
        sprintf(line, "Block stacks \"%s\" are not equal", result.detail.block_stacks_are_not_equal.label);
        break;
    case INIT_RET_OR_JMP_MUST_TERMINATE_BLOCK:
        sprintf(line, "Ret or Jmp must terminate block at %04x", result.detail.ret_must_terminate_block.ip);
        break;
    case INIT_RET_INVALID_STACK:
        sprintf(line, "Ret invalid stack at %04x", result.detail.ret_invalid_stack.ip);
        break;
    case INIT_INVALID_LABEL:
        sprintf(line, "Invalid label \"%s\"", result.detail.invalid_label.label);
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
        sprintf(line, "Invalid instruction %s at %04x", IOp_name(result.detail.invalid_instruction.instruction), result.detail.invalid_instruction.ip);
        break;
    case INTERPRET_DIVISION_BY_ZERO:
        sprintf(line, "Division by zero at %04x", result.detail.division_by_zero.ip);
        break;
    default:
        sprintf(line, "Unknown error %04x", result.code);
    }

    return strdup(line);
}
