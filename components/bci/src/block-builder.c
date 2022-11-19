#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "block-builder.h"

#include "memory.h"

#define BUFFER_TRANCHE 2

BlockBuilder *block_builder_new(void)
{
    BlockBuilder *sb = ALLOCATE(BlockBuilder, 1);
    sb->buffer = ALLOCATE(char, BUFFER_TRANCHE);
    sb->size = BUFFER_TRANCHE;
    sb->count = 0;

    return sb;
}

static void reserve_size(BlockBuilder *sb, int32_t count)
{
    if (sb->count + count >= sb->size)
    {
        int32_t new_buffer_size = sb->count + count + BUFFER_TRANCHE;
        sb->buffer = REALLOCATE(sb->buffer, char, new_buffer_size);
        sb->size = new_buffer_size;
    }
}

void block_builder_append(BlockBuilder *builder, Op op)
{
    reserve_size(builder, 1);
    builder->buffer[builder->count++] = op;
}

void block_builder_append_s32(BlockBuilder *builder, Op op, int32_t v) {
    reserve_size(builder, 1 + sizeof(int32_t));
    builder->buffer[builder->count++] = op;
    memcpy(builder->buffer + builder->count, &v, sizeof(int32_t));
    builder->count += sizeof(int32_t);    
}

Block *block_builder_build(BlockBuilder *builder)
{
    Block *block = block_new_populate(builder->buffer, builder->count);
    FREE(builder);

    return block;
}
