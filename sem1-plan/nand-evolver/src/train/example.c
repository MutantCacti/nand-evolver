/*
 * train/example.c
 * The Evolver's function at the Example level.
 *
 * An example is a lifetime: the Harness resets the Arena here, at its start,
 * and never between the rounds inside it. Then each round in order — the
 * Harness runs it, and on a graded round the Verifier turns the output region
 * into an error. The example's error and ticks travel up uncombined, because
 * only the Selector may combine per-example results.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

void example_measure(const Genome * genome, Arena * arena,
                     const Dataset * dataset,
                     const uint8_t * inputs, const uint8_t * expected,
                     const uint8_t * initial,
                     uint32_t * error, uint32_t * ticks)
{
    (void)genome; (void)arena; (void)dataset; (void)inputs; (void)expected;
    (void)initial; (void)error; (void)ticks;
    abort();    /* stub */
}
