/*
 * train/mutator.c
 * Mutator: a parent's genome copied with random changes.
 *
 * Turning on one of the inert Nands held in reserve is a neutral mutation,
 * because writes are applied in reverse index order and a new Nand has the
 * highest index, so it loses every collision until something downstream reads
 * it. That is why capacity above the live count is a parameter of the search
 * rather than an implementation detail.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

Genome * mutator_mutate(const Genome * parent, uint64_t seed,
                        uint32_t generation, uint32_t child)
{
    (void)parent; (void)seed; (void)generation; (void)child;
    abort();    /* stub */
}

