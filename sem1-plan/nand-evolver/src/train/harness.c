/*
 * train/harness.c
 * The lane Harness: packs WORD_BITS examples into the lanes of each word, drives the protocol, reaches the Verifier and Trainer only through hooks, and skips ungraded rounds.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include "../core/harness.h"
#include "../core/kernel.h"

#include <stdlib.h>

int lane_harness_individual(const Genome * genome, Arena * arena,
                            const Example * examples, size_t num_examples,
                            const HarnessHooks * hooks, uint32_t tick_limit,
                            Record * records_out)
{
    (void)genome; (void)arena; (void)examples; (void)num_examples;
    (void)hooks; (void)tick_limit; (void)records_out;
    abort();    /* stub */
}

int lane_harness_example(const Genome * genome, Arena * arena,
                         const Example * group, size_t group_size,
                         const HarnessHooks * hooks, uint32_t tick_limit,
                         Record * records_out)
{
    (void)genome; (void)arena; (void)group; (void)group_size;
    (void)hooks; (void)tick_limit; (void)records_out;
    abort();    /* stub */
}
