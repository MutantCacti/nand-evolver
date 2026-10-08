/*
 * train/dataset.c
 * The Dataset file, mapped read-only, and the example lookup.
 *
 * Three functions because the file is a resource: open it, look examples up,
 * close it. The lookup is pure in (seed, generation, position) — no order is
 * stored, nothing is shuffled, and *epoch* is not a concept here.
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

const uint8_t * dataset_example(const Dataset * dataset, uint64_t seed,
                                uint32_t generation, uint32_t position,
                                const uint8_t ** expected)
{
    (void)dataset; (void)seed; (void)generation; (void)position; (void)expected;
    abort();    /* stub */
}
