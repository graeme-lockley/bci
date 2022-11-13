#include <stdlib.h>

#include "memory.h"

static int memory_allocated_count = 0;

char *memory_alloc(int size, char *file, int line)
{
    memory_allocated_count += 1;
    return realloc(NULL, size);
}

void memory_free(void *ptr, char *file, int line)
{
    memory_allocated_count -= 1;
    (void)realloc(ptr, 0);
}

int memory_allocated(void)
{
    return memory_allocated_count;
}
