/*
 * train/mutator.c
 * Mutator: parents to children by random changes. Pure in (seed, generation, child), so a generation is reproducible however it was scheduled.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

Genome * mutator_mutate(const Config * config, const Genome * parent,
                        uint64_t seed, size_t generation, size_t child)
{
    (void)config; (void)parent; (void)seed; (void)generation; (void)child;
    abort();    /* stub */
}
