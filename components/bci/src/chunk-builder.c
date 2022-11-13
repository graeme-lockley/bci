#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "chunk-builder.h"

#include "memory.h"

#define BUFFER_TRANCHE 2

ChunkBuilder *chunk_builder_new(void)
{
    ChunkBuilder *sb = ALLOCATE(ChunkBuilder, 1);
    sb->buffer = ALLOCATE(char, BUFFER_TRANCHE);
    sb->size = BUFFER_TRANCHE;
    sb->count = 0;

    return sb;
}

static void reserve_size(ChunkBuilder *sb, int count)
{
    if (sb->count + count >= sb->size)
    {
        int new_buffer_size = sb->count + count + BUFFER_TRANCHE;
        sb->buffer = REALLOCATE(sb->buffer, char, new_buffer_size);
        sb->size = new_buffer_size;
    }
}

void chunk_builder_append(ChunkBuilder *builder, Op op)
{
    reserve_size(builder, 1);
    builder->buffer[builder->count++] = op;
}

Chunk *chunk_builder_build(ChunkBuilder *builder)
{
    Chunk *chunk = chunk_new_populate(builder->buffer, builder->count);
    FREE(builder);

    return chunk;
}
