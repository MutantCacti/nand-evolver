/*
 * train/rng.c
 * Stateless random streams, drawn from a seed and a position in the trees.
 *
 * One function. There is no generator object to advance, so no two levels can
 * contend for one and no result depends on the order draws were made in. That
 * is what makes a run identical however its work is divided between threads.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

uint64_t rng_value(uint64_t seed, const uint32_t * path, size_t depth)
{
    (void)seed; (void)path; (void)depth;
    abort();    /* stub */
}
