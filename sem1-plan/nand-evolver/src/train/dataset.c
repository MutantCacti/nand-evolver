/*
 * train/dataset.c
 * Maps the train Dataset file read-only and looks up examples. The lookup is pure in (seed, generation, position); no order is ever stored.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int dataset_open(Dataset * dataset, const char * path)
{
    (void)dataset; (void)path;
    abort();    /* stub */
}

void dataset_close(Dataset * dataset)
{
    (void)dataset;
    abort();    /* stub */
}

int dataset_example(const Dataset * dataset,
                    uint64_t seed, size_t generation, size_t position,
                    Example * example)
{
    (void)dataset; (void)seed; (void)generation; (void)position; (void)example;
    abort();    /* stub */
}
