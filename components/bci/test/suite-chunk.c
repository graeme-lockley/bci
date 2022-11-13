#include <stdio.h>

#include "../src/bci.h"
#include "../src/chunk-builder.h"
#include "../src/chunk.h"

#include "minunit.h"

static InterpretResult run_chunk(ChunkBuilder *builder)
{
    Chunk *chunk = chunk_builder_build(builder);

    VM *vm = bci_initVM_populate(chunk);

    InterpretResult result = bci_run(vm);

    bci_freeVM(vm);

    return result;
}

static char *op_add_32(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_ADD_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 37, "Expected 42 + -5 to be 37");

    return NULL;
}

static char *op_add_32_invalid_argument_type(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_PUSH_TRUE);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_ADD_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_INVALID_ARGUMENT_TYPES, "Expected invalid argument type error");
    mu_assert(result.detail.invalid_argument_types.instruction == OP_ADD_S32, "Expected instruction OP_ADD_S32");

    return NULL;
}

static char *op_sub_32(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_SUB_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 47, "Expected 42 - -5 to be 47");

    return NULL;
}

static char *op_sub_32_invalid_argument_type(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_PUSH_TRUE);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_SUB_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_INVALID_ARGUMENT_TYPES, "Expected invalid argument type error");
    mu_assert(result.detail.invalid_argument_types.instruction == OP_SUB_S32, "Expected instruction OP_SUB_S32");

    return NULL;
}

static char *op_mul_32(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_MUL_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == -210, "Expected 42 * -5 to be -210");

    return NULL;
}

static char *op_mul_32_invalid_argument_type(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_PUSH_TRUE);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_MUL_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_INVALID_ARGUMENT_TYPES, "Expected invalid argument type error");
    mu_assert(result.detail.invalid_argument_types.instruction == OP_MUL_S32, "Expected instruction OP_MUL_S32");

    return NULL;
}

static char *op_div_32(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_DIV_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == -8, "Expected 42 / -5 to be -8");

    return NULL;
}

static char *op_div_32_by_zero(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append_s32(builder, OP_PUSH_S32, 0);
    chunk_builder_append(builder, OP_DIV_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_DIVISION_BY_ZERO, "Expected division by zero error");

    return NULL;
}

static char *op_div_32_invalid_argument_type(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_PUSH_TRUE);
    chunk_builder_append_s32(builder, OP_PUSH_S32, -5);
    chunk_builder_append(builder, OP_DIV_S32);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_INVALID_ARGUMENT_TYPES, "Expected invalid argument type error");
    mu_assert(result.detail.invalid_argument_types.instruction == OP_DIV_S32, "Expected instruction OP_DIV_S32");

    return NULL;
}

static char *op_ret_with_empty_stack(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 0, "Expected ok result to be 0");

    return NULL;
}

static char *op_ret_with_value(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 42, "Expected ok result to be 42");

    return NULL;
}

char *suite_chunk(void)
{
    mu_run_test(op_ret_with_empty_stack);
    mu_run_test(op_ret_with_value);

    mu_run_test(op_add_32);
    mu_run_test(op_add_32_invalid_argument_type);
    mu_run_test(op_sub_32);
    mu_run_test(op_sub_32_invalid_argument_type);
    mu_run_test(op_mul_32);
    mu_run_test(op_mul_32_invalid_argument_type);
    mu_run_test(op_div_32);
    mu_run_test(op_div_32_by_zero);
    mu_run_test(op_div_32_invalid_argument_type);

    return NULL;
}
