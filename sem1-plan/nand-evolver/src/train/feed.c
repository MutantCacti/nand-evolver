/*
 * train/feed.c
 * train's Feed: the Dataset on the way in, the Verifier on the way out.
 *
 * The one file that knows both that examples come from a Dataset and that
 * output is scored. It packs a group of up to WORD_BITS examples into lanes,
 * reports an example boundary when the group advances, and on a graded round
 * calls the Verifier and accumulates a Record per example.
 *
 * Granularity lives here rather than in the Harness, so a Selector that ever
 * wants round-level error can have it without the Harness changing.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int feed_open(Feed * feed, const Config * config, const Dataset * dataset,
              uint64_t seed, size_t generation,
              size_t first_example, size_t num_examples,
              Record * records)
{
    (void)feed; (void)config; (void)dataset;
    (void)seed; (void)generation; (void)first_example; (void)num_examples;
    (void)records;
    abort();    /* stub */
}

void feed_close(Feed * feed)
{
    (void)feed;
    abort();    /* stub */
}

const Record * feed_records(const Feed * feed, size_t * count)
{
    (void)feed; (void)count;
    abort();    /* stub */
}
