#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "buffer.h"
#include "ebuilder.h"
#include "map.h"
#include "memory.h"

BlockBuilder *block_builder_new(void)
{
    return buffer_new(sizeof(char));
}

void block_builder_append(BlockBuilder *builder, EOp op)
{
    buffer_append(builder, &op, 1);
}

void block_builder_append_s32(BlockBuilder *builder, EOp op, int32_t v)
{
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
    if (builder)
    {
        buffer_free(builder);
    }
}

BlocksBuilder *blocks_builder_new(void)
{
    return map_string_new((void (*)(void *))block_builder_free);
}

BlockBuilder *blocks_builder_new_block(BlocksBuilder *builder, char *name)
{
    BlockBuilder *block_builder = (BlockBuilder *)map_get(builder, name);
    if (block_builder != NULL)
    {
        return block_builder;
    }

    block_builder = block_builder_new();
    map_add(builder, STRDUP(name), block_builder);

    return block_builder;
}

static void build_iterate(Blocks *blocks, Map_Node *node)
{
    if (node)
    {
        Block *block = block_builder_build((BlockBuilder *)node->value);
        map_add(blocks, node->key, block);
        node->key = NULL;
        node->value = NULL;

        build_iterate(blocks, node->left);
        build_iterate(blocks, node->right);
    }
}

Map *blocks_builder_build(BlocksBuilder *builder)
{
    Map *blocks = map_string_new((void (*)(void *))block_free);

    build_iterate(blocks, builder->root);

    blocks_builder_free(builder);

    return blocks;
}

void blocks_builder_free(BlocksBuilder *builder)
{
    map_free(builder);
}
