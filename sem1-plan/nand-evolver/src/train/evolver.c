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
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

static int evolver_generation(const Dataset * dataset, uint64_t seed,
                              uint32_t generation, Genome ** population,
                              uint32_t * error, uint32_t * ticks);
static int evolver_checkpoint(const char * out_dir, uint32_t generation,
                              Genome * const * population);

#if defined(TRAINING_EVOLVER_POPULATION)

/* Population mode: every (genome, example) pair is independent, so the split
 * is free. */
int evolver_run(const Dataset * dataset, uint64_t seed, const char * out_dir)
{
    (void)dataset; (void)seed; (void)out_dir;
    (void)evolver_generation; (void)evolver_checkpoint;
    abort();    /* stub */
}

#elif defined(TRAINING_EVOLVER_INDIVIDUAL)

/* Individual mode: the Trainer rewrites a genome between examples, so a
 * genome's examples are a sequence and must stay together and in order. The
 * loop is the same one; only the splitting rule changes, which is why this is
 * a variant of the Evolver and not a level of its own. */
int evolver_run(const Dataset * dataset, uint64_t seed, const char * out_dir)
{
    (void)dataset; (void)seed; (void)out_dir;
    (void)evolver_generation; (void)evolver_checkpoint;
    abort();    /* stub */
}

#else
#error "training.evolver names no Evolver this build provides"
#endif

static int evolver_generation(const Dataset * dataset, uint64_t seed,
                              uint32_t generation, Genome ** population,
                              uint32_t * error, uint32_t * ticks)
{
    (void)dataset; (void)seed; (void)generation;
    (void)population; (void)error; (void)ticks;
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
