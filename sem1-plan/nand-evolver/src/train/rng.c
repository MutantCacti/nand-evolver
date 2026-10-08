/*
 * train/rng.c
 * Stateless random streams derived from a seed and a position in the tree. Nothing is stored or advanced, so a run is independent of how its work was divided between threads.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

uint64_t rng_draw(uint64_t seed, const uint64_t * indices, size_t num_indices)
{
    (void)seed; (void)indices; (void)num_indices;
    abort();    /* stub */
}

uint64_t rng_below(uint64_t draw, uint64_t bound)
{
    (void)draw; (void)bound;
    abort();    /* stub */
}
