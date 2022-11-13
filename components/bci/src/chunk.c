#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

Chunk *chunk_new_populate(char *code, int32_t size)
{
    Chunk *chunk = ALLOCATE(Chunk, 1);

    chunk->code = code;
    chunk->size = size;

    return chunk;
}

void chunk_free(Chunk *chunk)
{
    FREE(chunk->code);

    chunk->code = NULL;
    chunk->size = 0;

    FREE(chunk);
}
