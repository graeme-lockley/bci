#ifndef BLOCK_H
#define BLOCK_H

#include "map.h"

typedef enum
{
    VT_S32,
    VT_BOOL
} ValueType;

typedef struct {
    char *code;
    int32_t size;
    ValueType *types;
    int32_t types_size;
    int verified;
} Block;

typedef Map Blocks;

extern Block *block_new_populate(char *code, int32_t size);
extern void block_free(Block *block);

extern void blocks_free(Blocks *blocks);

#endif
