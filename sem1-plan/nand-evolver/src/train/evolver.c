/*
 * train/evolver.c
 * Evolver (Run, Generation): generations to individuals. The one place work is split; owns checkpoints; hands the best genome to the Exporter at the end of a run.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

/* Generation: measure every individual, then Selector and then Mutator. */
static int evolver_generation(const Config * config, const Dataset * dataset,
                              uint64_t seed, size_t generation,
                              Population * population, Record * records);

/* Checkpoints are the Evolver's own: the population in training form at a
 * generation boundary, read back only here, never exported. */
static int evolver_checkpoint_write(const char * run_dir, size_t generation,
                                    const Population * population);
static int evolver_checkpoint_read(const char * run_dir,
                                   size_t * generation, Population * population);

int evolver_run(const Config * config, const Dataset * dataset,
                uint64_t seed, const char * run_dir)
{
    (void)config; (void)dataset; (void)seed; (void)run_dir;
    abort();    /* stub */
}

static int evolver_generation(const Config * config, const Dataset * dataset,
                       uint64_t seed, size_t generation,
                       Population * population, Record * records)
{
    (void)config; (void)dataset; (void)seed; (void)generation;
    (void)population; (void)records;
    abort();    /* stub */
}

static int evolver_checkpoint_write(const char * run_dir, size_t generation,
                             const Population * population)
{
    (void)run_dir; (void)generation; (void)population;
    abort();    /* stub */
}

static int evolver_checkpoint_read(const char * run_dir,
                            size_t * generation, Population * population)
{
    (void)run_dir; (void)generation; (void)population;
    abort();    /* stub */
}
