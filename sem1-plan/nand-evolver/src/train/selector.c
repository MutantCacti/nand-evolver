/*
 * train/selector.c
 * Selector: compares genomes, and owns every reduction over examples.
 *
 * It is handed per-(genome, example) results unreduced, because how they add
 * up is selection policy: a tournament on summed error and a lexicase
 * selection that needs each example separately are the same component with
 * different code here, and nothing above may have decided for them.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

void selector_select(const uint32_t * error, const uint32_t * ticks,
                     uint32_t num_genomes, uint32_t num_examples,
                     uint64_t seed, uint32_t generation,
                     uint32_t * parents, uint32_t num_parents)
{
    (void)error; (void)ticks; (void)num_genomes; (void)num_examples;
    (void)seed; (void)generation; (void)parents; (void)num_parents;
    abort();    /* stub */
}

