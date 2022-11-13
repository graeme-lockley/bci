#ifndef MEMORY_H
#define MEMORY_H

#define DEBUG_MEMORY

#include <stdlib.h>

#ifdef DEBUG_MEMORY

extern char *memory_alloc(int size, char *file, int line);
extern void memory_free(void *ptr, char *file, int line);
extern int memory_allocated(void);

#define ALLOCATE(type, count) \
    (type *)memory_alloc(sizeof(type) * (count), __FILE__, __LINE__)

#define REALLOCATE(pointer, type, count) \
    (type *)realloc(pointer, sizeof(type) * (count))

#define FREE(pointer) \
    memory_free(pointer, __FILE__, __LINE__)

#else

#define ALLOCATE(type, count) \
    (type *)realloc(NULL, sizeof(type) * (count))

#define REALLOCATE(pointer, type, count) \
    (type *)realloc(pointer, sizeof(type) * (count))

#define FREE(pointer) \
    ((void)realloc(pointer, 0))

#endif

#endif
