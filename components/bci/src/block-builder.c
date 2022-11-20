#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "buffer.h"
#include "block-builder.h"

#include "memory.h"


BlockBuilder *block_builder_new(void)
{
    return buffer_new(sizeof(char));
}

void block_builder_append(BlockBuilder *builder, EOp op)
{
    buffer_append(builder, &op, 1);
}

void block_builder_append_s32(BlockBuilder *builder, EOp op, int32_t v) {
    buffer_append(builder, &op, 1);
    buffer_append(builder, &v, sizeof(int32_t));
}

Block *block_builder_build(BlockBuilder *builder)
{
    int32_t size = buffer_count(builder);
    char *code = buffer_free_use(builder);
    Block *block = block_new_populate(code, size);

    return block;
}

void block_builder_free(BlockBuilder *builder)
{
    buffer_free(builder);
}
