#ifndef BLOCK_H
#define BLOCK_H

typedef struct {
    char *code;
    int32_t size;
} Block;

extern Block *block_new_populate(char *code, int32_t size);
extern void block_free(Block *block);

#endif
