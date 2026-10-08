/*
 * tests/test_layouts.c
 * Differential: the lane and packed Harness and Kernel agree.
 *
 * Both link into this binary, which is why their boundaries live in
 * core/harness.h and core/kernel.h. The same genome and the same examples run
 * through each, and every output region and tick count must match.
 *
 * The test supplies its own Feed, held in memory, which hands out the examples
 * and keeps every round's output for comparison. That is why neither Harness
 * needs a public per-example entry, and why no Verifier or Trainer is involved:
 * a feed is the only thing either Harness can see.
 *
 * Both Kernels must also keep their #if branches in the same order, or this
 * test compares two different schemes and passes for the wrong reason.
 *
 * This is the test that makes 'the Harness is the same component in both
 * programs' a checked claim rather than an intention.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */

#include <stdlib.h>

int main(void)
{
    abort();    /* stub */
}
