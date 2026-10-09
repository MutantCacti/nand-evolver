/*
 * train/evolver.c
 * Evolver (Run, Generation): the search, and the one place work is divided.
 *
 * It flattens a generation into (genome, example) pairs and splits them. The
 * piece of work it hands out is one genome and a range of examples: a range of
 * one is the reference. Nothing in a work unit reads another's state, so they
 * can be measured in any order and in parallel, and an Arena is a worker's
 * scratch reused from one unit to the next.
 *
 * There is no Individual level. A genome and the memory space it is measured
 * with are not one thing — the genome is population state, owned here, and the
 * memory space is cleared at every example boundary, which is what an example
 * already means.
 *
 * Per work unit it calls harness_example once per example, then the Verifier
 * on each graded round of what came back. Both the Verifier and (in P2) the
 * Trainer are leaves called from here, so the Harness never learns that
 * grading exists.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

/* One generation: measure every (genome, example) pair, then select and
 * mutate. The per-pair error and ticks are scratch owned here, not arguments:
 * nothing above a generation reads them, because the Selector consumes them
 * unreduced and the Logger summarises them, both before this returns. Ticks
 * are not a separate kind of thing from error either — both are per-pair
 * results the Selector weighs, so both have the same lifetime.
 *
 * What travels up is one number: the index the Selector ranked first. Not its
 * error, because error is not what "best" means once the Selector weighs
 * ticks against it, and under a policy like lexicase there is no single score
 * to compare at all — that is the reason results reach it unreduced.
 *
 * One number is enough because every generation preserves its best: the
 * genome ranked first reaches the next population unmutated, so by induction
 * the best of the last generation measured is the best of the run. The Evolver
 * keeps one copy and replaces it every generation — it must copy, because the
 * Mutator overwrites `population` — and never compares two generations, which
 * is just as well, since under lexicase it would have nothing to compare. */
static int evolver_generation(const Dataset * dataset, uint64_t seed,
                              uint32_t generation, Genome ** population,
                              uint32_t * best);
static int evolver_checkpoint(const char * out_dir, uint32_t generation,
                              Genome * const * population);

/* Every (genome, example) pair is independent, so the split is free. The
 * variant that orders a genome's examples so a Trainer can rewrite it between
 * them is P2, and arrives with the key that selects it. */
int evolver_run(const Dataset * dataset, uint64_t seed, const char * out_dir)
{
    (void)dataset; (void)seed; (void)out_dir;
    (void)evolver_generation; (void)evolver_checkpoint;
    abort();    /* stub */
}

static int evolver_generation(const Dataset * dataset, uint64_t seed,
                              uint32_t generation, Genome ** population,
                              uint32_t * best)
{
    (void)dataset; (void)seed; (void)generation;
    (void)population; (void)best;
    abort();    /* stub */
}

/* The population in training form at a generation boundary, where the whole
 * population is at rest. Read back only here, to resume a run; never exported. */
static int evolver_checkpoint(const char * out_dir, uint32_t generation,
                              Genome * const * population)
{
    (void)out_dir; (void)generation; (void)population;
    abort();    /* stub */
}
