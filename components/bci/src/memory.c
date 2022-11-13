#include <stdlib.h>

#include "memory.h"

static int32_t memory_allocated_count = 0;

char *memory_alloc(int32_t size, char *file, int32_t line)
{
    memory_allocated_count += 1;
    return realloc(NULL, size);
}

void memory_free(void *ptr, char *file, int32_t line)
{
    memory_allocated_count -= 1;
    (void)realloc(ptr, 0);
}

int32_t memory_allocated(void)
{
    return memory_allocated_count;
}
