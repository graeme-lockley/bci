#ifndef BLOCK_BUILDER_H
#define BLOCK_BUILDER_H

#include "block.h"
#include "buffer.h"
#include "op.h"

typedef Buffer BlockBuilder;

extern BlockBuilder *block_builder_new(void);

extern void block_builder_append(BlockBuilder *builder, Op op);
extern void block_builder_append_s32(BlockBuilder *builder, Op op, int32_t v);
    
extern Block *block_builder_build(BlockBuilder *builder);
extern void block_builder_free(BlockBuilder *builder);

#endif
