#ifndef CHUNK_H
#define CHUNK_H

typedef struct {
    char *code;
    int size;
} Chunk;

extern Chunk *chunk_new_populate(char *code, int size);
extern void chunk_free(Chunk *chunk);

#endif
