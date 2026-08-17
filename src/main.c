/*
 * main.c
 * Entry point / arg parser
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#include <stdio.h>
#include <string.h>
#include "model.h"


const char *DEFAULT_OUTPUT = "model";


static void print_usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [--output \"best\"]",
        prog
    );
}


int main(int argc, const char *argv[])
{
    const char *output_str = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0) && i + 1 < argc) {
            output_str = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!output_str) {
        output_str = DEFAULT_OUTPUT;
    }

    fprintf(stdout, "output_str=%s\n", output_str);


    Model *model = model_init(2);
    Nand *nand = nand_init(0, 1, 1);
    model_add_nand(model, nand);

    fprintf(stdout, "%d, %d\n", model->arena[0], model->arena[1]);
    for (int i = 0; i < 10; i++)
    {
        model_compute_ticks(model, 1);
        fprintf(stdout, "%d, %d\n", model->arena[0], model->arena[1]);
    }

    model_free(model);
    return 0;
}