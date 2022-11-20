#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/block.h"
#include "../src/block-builder.h"
#include "../src/bci.h"

#include "minunit.h"

static char *trim(char *input)
{
    int length_of_input = strlen(input);

    if (length_of_input == 0)
        return input;

    int start_of_input = 0;
    while (start_of_input < length_of_input && input[start_of_input] < 33)
        start_of_input += 1;

    int end_of_input = length_of_input - 1;
    while (end_of_input > 0 && input[end_of_input] < 33)
        end_of_input -= 1;

    if (start_of_input == length_of_input)
    {
        free(input);
        return strdup("");
    }
    else if (start_of_input == 0 && end_of_input == length_of_input - 1)
    {
        return input;
    }
    else
    {
        int length_of_result = end_of_input - start_of_input + 1;

        char *result = malloc(length_of_result + 1);
        strncpy(result, input + start_of_input, length_of_result);
        result[length_of_result] = '\0';
        free(input);
        return result;
    }
}

static char *trim_preserve(char *input)
{
    return trim(strdup(input));
}

static char *run_test(char *scenario, Block *block, char *expected_result)
{
    mu_run_test_label_start(scenario);

    VM *vm = bci_initVM_populate(block);
    InterpretResult result = bci_run(vm);
    bci_freeVM(vm);

    char *r = bci_interpretResult_toString(result);
    mu_assert(strcmp(r, expected_result) == 0, r);
    free(r);

    mu_run_test_label_end();

    return NULL;
}

void append(BlockBuilder *cb, char *trimmed)
{
    if (strcmp(trimmed, "ADD_S32") == 0)
    {
        block_builder_append(cb, OP_ADD_S32);
    }
    else if (strcmp(trimmed, "DIV_S32") == 0)
    {
        block_builder_append(cb, OP_DIV_S32);
    }
    else if (strcmp(trimmed, "MUL_S32") == 0)
    {
        block_builder_append(cb, OP_MUL_S32);
    }
    else if (strcmp(trimmed, "SUB_S32") == 0)
    {
        block_builder_append(cb, OP_SUB_S32);
    }
    else if (strcmp(trimmed, "PUSH_FALSE") == 0)
    {
        block_builder_append(cb, OP_PUSH_FALSE);
    }
    else if (strcmp(trimmed, "PUSH_TRUE") == 0)
    {
        block_builder_append(cb, OP_PUSH_TRUE);
    }
    else if (strcmp(trimmed, "RET") == 0)
    {
        block_builder_append(cb, OP_RET);
    }
    else if (strcmp(trimmed, "RET_BOOL") == 0)
    {
        block_builder_append(cb, OP_RET_BOOL);
    }
    else if (strcmp(trimmed, "RET_S32") == 0)
    {
        block_builder_append(cb, OP_RET_S32);
    }
    else if (strncmp(trimmed, "PUSH_S32 ", 8) == 0)
    {
        block_builder_append_s32(cb, OP_PUSH_S32, atoi(trimmed + 9));
    }
    else
    {
        printf("Unknown instruction: [%s]\n", trimmed);
    }
}

char *test_file(char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        return "File not found";
    }

    // 0: looking for start of next test or end of file
    // 1: adding code to test scenario and looking for result
    int test_state = 0;

    char *test_scenario = NULL;
    BlockBuilder *cb = NULL;
    while (1)
    {
        char *line = NULL;
        size_t len = 0;
        ssize_t read = getline(&line, &len, fp);

        if (read == -1)
        {
            if (cb != NULL)
            {
                block_builder_free(cb);
            }
            fclose(fp);
            return NULL;
        }

        line = trim(line);

        if (line[0] == '\0' || line[0] == '#')
        {
            // Do nothing
        }
        else if (line[0] == '.' && test_state == 0)
        {
            test_scenario = trim_preserve(line + 1);
            cb = block_builder_new();
            test_state = 1;
        }
        else if (line[0] == '>')
        {
            if (test_state == 1)
            {
                char *expected_result = trim_preserve(line + 1);
                test_state = 0;
                Block *block = block_builder_build(cb);
                cb = NULL;

                char *result = run_test(test_scenario, block, expected_result);

                free(test_scenario);
                test_scenario = NULL;
                free(expected_result);
                expected_result = NULL;

                if (result != NULL)
                {
                    fclose(fp);
                    return result;
                }
            }
            else
            {
                printf("Unexpected >\n");
            }
        }
        else if (test_state == 1)
        {
            append(cb, line);
        }

        free(line);
    }
}
