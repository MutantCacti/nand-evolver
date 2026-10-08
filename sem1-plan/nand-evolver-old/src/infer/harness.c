/*
 * infer/harness.c
 * The packed Harness: one example, one bit per wire.
 *
 * One public function, because reading input and emitting output are the feed's
 * job: deployment hands it a feed over stdin and stdout, and test_layouts hands
 * it one held in memory. The Deployment, Example and Round levels are each one
 * private function, exactly as in the lane Harness.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/harness.h"
#include "../core/kernel.h"

#include <stdlib.h>

static FeedStatus packed_harness_example(const Model * model, Arena * arena,
                                         const Feed * feed, uint32_t tick_limit);

static int packed_harness_round(const Model * model, Arena * arena,
                                const Feed * feed, uint32_t tick_limit,
                                const word * inputs);

int packed_harness_run(const Model * model, Arena * arena,
                       const Feed * feed, uint32_t tick_limit)
{
    (void)model; (void)arena; (void)feed; (void)tick_limit;
    abort();    /* stub */
}

static FeedStatus packed_harness_example(const Model * model, Arena * arena,
                                         const Feed * feed, uint32_t tick_limit)
{
    (void)model; (void)arena; (void)feed; (void)tick_limit;
    abort();    /* stub */
}

static int packed_harness_round(const Model * model, Arena * arena,
                                const Feed * feed, uint32_t tick_limit,
                                const word * inputs)
{
    (void)model; (void)arena; (void)feed; (void)tick_limit; (void)inputs;
    abort();    /* stub */
}
