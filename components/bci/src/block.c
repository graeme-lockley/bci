#include <stdlib.h>

#include "block.h"
#include "memory.h"

Block *block_new_populate(char *code, int32_t size)
{
    Block *block = ALLOCATE(Block, 1);

    block->code = code;
    block->size = size;

    block->types = NULL;
    block->types_size = -1;

    block->verified = 0;

    return block;
}

void block_free(Block *block)
{
    FREE(block->code);

    block->code = NULL;
    block->size = 0;

    if (block->types)
    {
        FREE(block->types);
        block->types = NULL;
        block->types_size = -1;
    }

    FREE(block);
}

void blocks_free(Blocks *blocks)
{
    map_free(blocks);
}
