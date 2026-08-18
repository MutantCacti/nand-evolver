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
    Model *m = model_init(3);
    print_arena(m);

    test_init_input_listener("/dev/input/event3");

    for (;;)
    {
        printf("%d\n", test_read_error(m));
        usleep(100000);
    }

    return 0;
}
