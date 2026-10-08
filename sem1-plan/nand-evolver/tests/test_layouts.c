/*
 * tests/test_layouts.c
 * Differential: the lane and packed Harness and Kernel agree.
 *
 * Both link into this binary, which is why their boundaries live in
 * core/harness.h and core/kernel.h. The same genome and the same examples run
 * through each, and every output region and tick count must match. Hooks are
 * stubbed, so no Verifier or Trainer is involved.
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
