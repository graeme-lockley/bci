#ifndef BLOCK_BUILDER_H
#define BLOCK_BUILDER_H

#include "block.h"
#include "op.h"

typedef struct
{
    char *buffer;
    int32_t size;
    int32_t count;
} BlockBuilder;

extern BlockBuilder *block_builder_new(void);

extern void block_builder_append(BlockBuilder *builder, Op op);
extern void block_builder_append_s32(BlockBuilder *builder, Op op, int32_t v);
    
extern Block *block_builder_build(BlockBuilder *builder);

#endif
