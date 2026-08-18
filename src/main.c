/*
 * main.c
 * Entry point
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#include <stdio.h>
#include <string.h>
#include "test.h"


void print_arena(Model *model)
{
    for (size_t i = 0; i < model->arena_size; i++)
    {
        word w = model->arena[word_index(i)];
        word bit = (w >> word_offset(i)) & (word)1;
        printf("%lu", bit);
    }
    fprintf(stdout, "\n");
}


int main(void)
{
    Model *model = model_init(2);
    model_add_nand(model, 0, 1, 1);

    print_arena(model);
    for (int i = 0; i < 5; i++)
    {
        model_compute_ticks(model, 1);
        print_arena(model);
    }

    model_update_arena_size(model, 3);
    model_add_nand(model, 1, 2, 2);

    printf("---\n");
    for (int i = 0; i < 5; i++)
    {
        model_compute_ticks(model, 1);
        print_arena(model);
    }

    model_remove_nand(model, 0);
    printf("---\n");
    for (int i=0; i < 5; i++)
    {
        model_compute_ticks(model, 1);
        print_arena(model);
    }

    model_free(model);
    return 0;
}
