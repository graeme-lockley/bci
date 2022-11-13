#include <stdio.h>

#include "../src/bci.h"
#include "../src/chunk-builder.h"
#include "../src/chunk.h"

#include "minunit.h"

char *suite_chunk(void)
{
    ChunkBuilder *builder = chunk_builder_new();

    chunk_builder_append(builder, OP_RET);

    Chunk *chunk = chunk_builder_build(builder);

    VM *vm = bci_initVM_populate(chunk);

    InterpretResult result = bci_run(vm);

    bci_freeVM(vm);

    mu_assert(result.code == INTERPRET_OK, "Expected no error");
    mu_assert(result.detail.ok.result == 0, "Expected ok result to be 0");

    return NULL;
}
