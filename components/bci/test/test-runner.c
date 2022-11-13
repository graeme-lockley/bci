#include <stdio.h>

#include "../src/memory.h"
#include "minunit.h"

#define TEST_SUITE(name) \
    if (result == NULL)  {\
        char * name(void); \
        printf(". Running %s\n", #name); \
        result = name(); \
    } else { \
        printf(". Skipping %s\n", #name); \
    }

int main(void)
{
    char *result = NULL;

#ifdef DEBUG_MEMORY
    printf(". Memory allocated delta: %d\n", memory_allocated());
#endif

    TEST_SUITE(suite_chunk);

    if (result == NULL)
    {
        printf(". All tests passed\n");
    }
    else
    {
        printf(". Failed: %s\n", result);
    }

    printf(". Run: %d   Passed: %d   Failed: %d\n", tests_run, tests_passed, (tests_run - tests_passed));

#ifdef DEBUG_MEMORY
    printf(". Memory allocated delta: %d\n", memory_allocated());
#endif

    return result != NULL;
}
