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

static char *op_ret_with_empty_stack(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_RET);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 0, "Expected ok result to be 0");

    return NULL;
}

static char *op_ret_s32_with_value(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append_s32(builder, OP_PUSH_S32, 42);
    chunk_builder_append(builder, OP_RET_S32);

    InterpretResult result = run_chunk(builder);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 42, "Expected ok result to be 42");

    return NULL;
}

char *suite_chunk(void)
{
    mu_run_test(op_ret_with_empty_stack);
    mu_run_test(op_ret_s32_with_value);

    return NULL;
}
