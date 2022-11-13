#ifndef CHUNK_BUILDER_H
#define CHUNK_BUILDER_H

#include "chunk.h"
#include "op.h"

typedef struct
{
    char *buffer;
    int size;
    int count;
} ChunkBuilder;

extern ChunkBuilder *chunk_builder_new(void);
extern void chunk_builder_append(ChunkBuilder *builder, Op op);
extern Chunk *chunk_builder_build(ChunkBuilder *builder);

#endif
