#ifndef EBUILDER_H
#define EBUILDER_H

#include "block.h"
#include "buffer.h"
#include "op.h"

typedef Buffer BlockBuilder;

extern BlockBuilder *block_builder_new(void);

extern void block_builder_append(BlockBuilder *builder, EOp op);
extern void block_builder_append_s32(BlockBuilder *builder, EOp op, int32_t v);
    
extern Block *block_builder_build(BlockBuilder *builder);
extern void block_builder_free(BlockBuilder *builder);

#endif
