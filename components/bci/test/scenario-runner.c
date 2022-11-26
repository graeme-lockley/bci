#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/block.h"
#include "../src/ebuilder.h"
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

static char *run_test(char *scenario, Blocks *blocks, char *expected_result)
{
    mu_run_test_label_start(scenario);

    InitResult initResult = bci_initVM_populate(blocks);

    char *r;

    if (initResult.code == INIT_OK)
    {
        InterpretResult result = bci_run(initResult.detail.ok.vm);
        bci_freeVM(initResult.detail.ok.vm);
        r = bci_interpretResult_toString(result);
    }
    else
    {
        r = bci_initResult_toString(initResult);
    }

    mu_assert(strcmp(r, expected_result) == 0, r);
    free(r);

    mu_run_test_label_end();

    return NULL;
}

int append(BlocksBuilder *blocks_builder, BlockBuilder **cb, char *trimmed)
{
    if (strcmp(trimmed, "ADD_S32") == 0)
    {
        block_builder_append(*cb, EOP_ADD_S32);
    }
    else if (strcmp(trimmed, "DIV_S32") == 0)
    {
        block_builder_append(*cb, EOP_DIV_S32);
    }
    else if (strcmp(trimmed, "MUL_S32") == 0)
    {
        block_builder_append(*cb, EOP_MUL_S32);
    }
    else if (strcmp(trimmed, "SUB_S32") == 0)
    {
        block_builder_append(*cb, EOP_SUB_S32);
    }
    else if (strcmp(trimmed, "PUSH_FALSE") == 0)
    {
        block_builder_append(*cb, EOP_PUSH_FALSE);
    }
    else if (strcmp(trimmed, "PUSH_TRUE") == 0)
    {
        block_builder_append(*cb, EOP_PUSH_TRUE);
    }
    else if (strcmp(trimmed, "RET") == 0)
    {
        block_builder_append(*cb, EOP_RET);
    }
    else if (strcmp(trimmed, "RET_BOOL") == 0)
    {
        block_builder_append(*cb, EOP_RET_BOOL);
    }
    else if (strcmp(trimmed, "RET_S32") == 0)
    {
        block_builder_append(*cb, EOP_RET_S32);
    }
    else if (strncmp(trimmed, "PUSH_S32 ", 8) == 0)
    {
        block_builder_append_s32(*cb, EOP_PUSH_S32, atoi(trimmed + 9));
    }
    else if (strncmp(trimmed, "JMP ", 4) == 0)
    {
        block_builder_append_string(*cb, EOP_JMP, trimmed + 4);
    }
    else if (strncmp(trimmed, "BLOCK ", 6) == 0)
    {
        *cb = blocks_builder_new_block(blocks_builder, trimmed + 6);
    }
    else
    {
        printf("Unknown instruction: [%s]\n", trimmed);
        return 0;
    }

    return 1;
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
    BlocksBuilder *blocks_builder = NULL;
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
                blocks_builder_free(blocks_builder);
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
            blocks_builder = blocks_builder_new();
            cb = blocks_builder_new_block(blocks_builder, "main");
            test_state = 1;
        }
        else if (line[0] == '>')
        {
            if (test_state == 1)
            {
                char *expected_result = trim_preserve(line + 1);
                test_state = 0;
                Blocks *blocks = blocks_builder_build(blocks_builder);
                blocks_builder = NULL;
                cb = NULL;

                char *result = run_test(test_scenario, blocks, expected_result);

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
            if (!append(blocks_builder, &cb, line)) {
                fclose(fp);
                free(line);
                blocks_builder_free(blocks_builder);
                return "Unknown instruction";
            }
        }

        free(line);
    }
}
