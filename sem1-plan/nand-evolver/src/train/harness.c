/*
 * train/harness.c
 * The lane Harness: WORD_BITS examples in the lanes of each word.
 *
 * The Individual, Example and Round levels are each one function, all private:
 * only the level above calls them, and a test drives the whole thing through a
 * Feed instead. The Trainer is the one place a training choice reaches in here,
 * and it is an #if rather than a callback, so the reference build contains no
 * trainer code at all.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include "../core/harness.h"
#include "../core/kernel.h"

#include <stdlib.h>

/* Example: clear the arena, then run rounds until the feed says the example is
 * over. Returns the status that ended it. */
static FeedStatus lane_harness_example(Genome * genome, Arena * arena,
                                       const Feed * feed, uint32_t tick_limit);

/* Round: write inputs and ready's start value, run the Kernel, hand the output
 * region and the tick count to the feed. */
static int lane_harness_round(Genome * genome, Arena * arena,
                              const Feed * feed, uint32_t tick_limit,
                              const word * inputs);

int lane_harness_run(Genome * genome, Arena * arena,
                     const Feed * feed, uint32_t tick_limit)
{
    (void)genome; (void)arena; (void)feed; (void)tick_limit;
    abort();    /* stub */
}

static FeedStatus lane_harness_example(Genome * genome, Arena * arena,
                                       const Feed * feed, uint32_t tick_limit)
{
    (void)genome; (void)arena; (void)feed; (void)tick_limit;

#if defined(TRAINING_TRAINER_NONE)
    /* No Trainer: an example boundary is only an arena clear. */
#else
    /* Individual mode: the example boundary is the one moment a genome may
     * change mid-measurement. The Trainer takes its evidence from the feed, so
     * the Harness still knows nothing of grading or error. */
    (void)trainer_train;
#endif

    abort();    /* stub */
}

static int lane_harness_round(Genome * genome, Arena * arena,
                              const Feed * feed, uint32_t tick_limit,
                              const word * inputs)
{
    (void)genome; (void)arena; (void)feed; (void)tick_limit; (void)inputs;
    abort();    /* stub */
}
