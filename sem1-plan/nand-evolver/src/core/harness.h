/*
 * core/harness.h
 * Harness: examples to rounds, driving the protocol the README defines.
 *
 * Per round it writes the input region and ready's start value, has the Kernel
 * run the genome, and reads the output region back. That is the whole of its
 * job, and it is the same job in both programs.
 *
 * Where a round's input comes from, and where its output goes, is the Feed's
 * business and never the Harness's. So the Harness knows nothing of datasets,
 * grading, error, stdin or stdout: train supplies a Feed over the Dataset whose
 * write runs the Verifier, deployment supplies one over stdin and stdout, and a
 * test supplies one held in memory. One public function per layout.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_HARNESS_H_
#define CORE_HARNESS_H_

#include "arena.h"
#include "genome.h"
#include "model.h"
#include "word.h"

#include <stddef.h>
#include <stdint.h>

/* What a read yielded. An **example** is the unit the arena is cleared for, so
 * in the lane layout it is a group of up to WORD_BITS examples advancing in
 * lockstep, not a single one. */
typedef enum
{
    FEED_ROUND = 0,     /* inputs filled: run this round of the current example */
    FEED_EXAMPLE,       /* inputs filled, and they open a new example: clear first */
    FEED_END            /* nothing left to run */
}
FeedStatus;

/* The Harness's only window onto the world. */
typedef struct
{
    /* Fill the next round's input values. */
    FeedStatus (*read)(void * context, word * inputs, size_t num_inputs);

    /* Take this round's output values, once the Kernel has stopped, with the
     * ticks it took. Whether that is scored, printed or stored is the feed's
     * business: train's write runs the Verifier on graded rounds, deployment's
     * writes a record to stdout, a test's keeps it for comparison. */
    int (*write)(void * context, const word * outputs, size_t num_outputs,
                 uint32_t ticks);

    void * context;
}
Feed;

/* Run until the feed ends.
 *
 * The genome is mutable because individual mode rewrites it at an example
 * boundary; nothing else here changes it. The Model is not, because the
 * deployed program never changes what it was given. */
int lane_harness_run(Genome * genome, Arena * arena,
                     const Feed * feed, uint32_t tick_limit);

int packed_harness_run(const Model * model, Arena * arena,
                       const Feed * feed, uint32_t tick_limit);

#endif
