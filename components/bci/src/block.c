#include <stdlib.h>

#include "block.h"
#include "memory.h"

Block *block_new_populate(char *code, int32_t size)
{
    Block *block = ALLOCATE(Block, 1);

    block->code = code;
    block->size = size;

    return block;
}

void block_free(Block *block)
{
    FREE(block->code);

    block->code = NULL;
    block->size = 0;

    FREE(block);
}
