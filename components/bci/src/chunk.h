#ifndef CHUNK_H
#define CHUNK_H

typedef struct {
    char *code;
    int32_t size;
} Chunk;

extern Chunk *chunk_new_populate(char *code, int32_t size);
extern void chunk_free(Chunk *chunk);

#endif
