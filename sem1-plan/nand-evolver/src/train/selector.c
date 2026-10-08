/*
 * train/selector.c
 * Selector: compares individuals and chooses parents. Owns every reduction over examples, because how per-example records add up to one comparison is a selection decision.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int selector_select(const Config * config,
                    const Record * records,
                    size_t num_individuals, size_t num_examples,
                    uint64_t seed, size_t generation,
                    size_t * parents, size_t num_parents)
{
    (void)config; (void)records; (void)num_individuals; (void)num_examples;
    (void)seed; (void)generation; (void)parents; (void)num_parents;
    abort();    /* stub */
}
