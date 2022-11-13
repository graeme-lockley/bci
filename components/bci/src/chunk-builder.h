#ifndef CHUNK_BUILDER_H
#define CHUNK_BUILDER_H

#include "chunk.h"
#include "op.h"

typedef struct
{
    char *buffer;
    int32_t size;
    int32_t count;
} ChunkBuilder;

extern ChunkBuilder *chunk_builder_new(void);

extern void chunk_builder_append(ChunkBuilder *builder, Op op);
extern void chunk_builder_append_s32(ChunkBuilder *builder, Op op, int32_t v);
    
extern Chunk *chunk_builder_build(ChunkBuilder *builder);

#endif
