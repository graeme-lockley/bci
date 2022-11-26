#ifndef EBUILDER_H
#define EBUILDER_H

#include "block.h"
#include "buffer.h"
#include "map.h"
#include "op.h"

typedef Buffer BlockBuilder;

extern BlockBuilder *block_builder_new(void);
extern void block_builder_append(BlockBuilder *builder, EOp op);
extern void block_builder_append_s32(BlockBuilder *builder, EOp op, int32_t v);
extern void block_builder_append_string(BlockBuilder *builder, EOp op, char *v);
extern Block *block_builder_build(BlockBuilder *builder);
extern void block_builder_free(BlockBuilder *builder);

typedef Map BlocksBuilder;

extern BlocksBuilder *blocks_builder_new(void);
extern BlockBuilder *blocks_builder_new_block(BlocksBuilder *builder, char *name);
extern Map *blocks_builder_build(BlocksBuilder *builder);
extern void blocks_builder_free(BlocksBuilder *builder);

#endif
