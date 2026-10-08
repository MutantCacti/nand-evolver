/*
 * train/train.h
 * The boundary functions of train's components, for main.c and the tests.
 *
 * One header for the whole of train, because these are shared boundaries
 * inside one program rather than between the two. Everything else in each .c
 * file is static: private functions do not exist until they are needed.
 *
 * Parameters arrive here at runtime, in Config. Protocol and training choices
 * arrive as #defines from build/<name>/config.h and are never fields.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef TRAIN_TRAIN_H_
#define TRAIN_TRAIN_H_

#include "../core/arena.h"
#include "../core/genome.h"
#include "../core/harness.h"
#include "../core/model.h"
#include "../core/word.h"

#include <stddef.h>
#include <stdint.h>

/* ---- config.c ---------------------------------------------------------- */

/* The runtime half of an experiment file. Structural choices are #defines, so
 * only numbers live here. Grows with the experiment-file schema.
 *
 * Two hashes, because two different things need identifying. The experiment
 * hash covers protocol, training, inference, parameter and task keys, but not
 * replicate or execution: it identifies results, is recorded by every run
 * directory, and is what the determinism test holds fixed. The build hash adds
 * the compile-time execution keys: it identifies this binary. */
typedef struct
{
    const char * name;          /* the experiment file's own filename, less .cfg */
    uint64_t experiment_hash;
    uint64_t build_hash;

    size_t population;
    unsigned generations;
    unsigned tick_limit;
    size_t address_slack;       /* wire space beyond what the genome drives */

    unsigned threads;           /* execution.threads, read at start-up */
}
Config;

/* Read a flat key = value experiment file, execution keys included. Refuses one
 * whose build hash differs from the hash compiled into this binary. */
int config_load(Config * config, const char * path);

/* ---- dataset.c --------------------------------------------------------- */

/* The train split, mapped read-only. Never changed after the Driver wrote it,
 * so every thread reads it without copying. */
typedef struct
{
    const void * bytes;
    size_t size;

    size_t num_examples;
    size_t num_inputs, num_outputs;
    size_t rounds_per_example;
}
Dataset;

int dataset_open(Dataset * dataset, const char * path);
void dataset_close(Dataset * dataset);

/* Example lookup, pure in (seed, generation, position): the same arguments
 * always name the same example, whatever order the work ran in. */
int dataset_example(const Dataset * dataset,
                    uint64_t seed, size_t generation, size_t position,
                    Example * example);

/* ---- rng.c ------------------------------------------------------------- */

/* A draw is a function of the seed and the position in the tree, e.g.
 * (generation, individual, example). No generator is stored or advanced, which
 * is what makes a run independent of how its work was divided. */
uint64_t rng_draw(uint64_t seed, const uint64_t * indices, size_t num_indices);

/* Uniform in [0, bound) from one draw, without modulo bias. */
uint64_t rng_below(uint64_t draw, uint64_t bound);

/* ---- evolver.c --------------------------------------------------------- */

/* One generation's individuals: a genome and its memory space each. */
typedef struct
{
    Genome ** genomes;
    Arena ** arenas;
    size_t count;
}
Population;

/* Run: generations from one seed, then hand the best genome to the Exporter.
 * The one place work is split, so threads enter and leave here. How many is
 * config->threads, an execution key, not a command-line argument. */
int evolver_run(const Config * config, const Dataset * dataset,
                uint64_t seed, const char * run_dir);

/* The Generation level is one function too, and so is each half of
 * checkpointing, but all three are private to evolver.c: only the run calls
 * them. A test resumes through evolver_run itself, by running to a generation
 * count and then running again to a higher one, since both are parameters. */

/* ---- selector.c -------------------------------------------------------- */

/* Compares individuals and chooses parents. Owns every reduction over
 * examples: records arrive per (individual, example) and uncombined, because
 * how they add up to one comparison is a selection decision. */
int selector_select(const Config * config,
                    const Record * records,
                    size_t num_individuals, size_t num_examples,
                    uint64_t seed, size_t generation,
                    size_t * parents, size_t num_parents);

/* ---- mutator.c --------------------------------------------------------- */

/* A child from one parent, by random changes: adding a Nand, rewiring an
 * index, reordering. Pure in (seed, generation, child). */
Genome * mutator_breed(const Config * config, const Genome * parent,
                       uint64_t seed, size_t generation, size_t child);

/* ---- trainer.c --------------------------------------------------------- */

/* Changes a genome between examples, from the results so far. Individual mode
 * only; installed as HarnessHooks.train, so this matches that signature. */
int trainer_train(void * context, Genome * genome,
                  const Record * records, size_t count);

/* ---- verifier.c -------------------------------------------------------- */

/* Produced against expected output wires on a graded round. Bitwise, so
 * wrong_out shows which wires are wrong as evidence for the Mutator and
 * Trainer. Installed as HarnessHooks.verify, so this matches that signature. */
uint32_t verifier_verify(void * context,
                         const word * produced, const word * expected,
                         size_t num_outputs, word active_lanes,
                         word * wrong_out);

/* ---- exporter.c -------------------------------------------------------- */

/* Canonicalise the best genome the run found, in the configured way, and write
 * it as the model file. Called once by the Evolver, at the end of a run. Data
 * leaves a run in one direction: Evolver -> Exporter -> model file. */
int exporter_export(const Config * config, const Genome * best,
                    const word * initial_state, size_t num_initial_words,
                    const char * path);

/* ---- logger.c ---------------------------------------------------------- */

/* The run log the Driver reads for reports and plots. Every component writes
 * its own events. Per-thread buffers, merged in canonical order at generation
 * boundaries, so logging never changes a result. */
typedef struct Logger Logger;

Logger * logger_open(const char * path, unsigned threads);
void logger_event(Logger * logger, unsigned thread, const char * event, const char * fields);
int logger_merge(Logger * logger, size_t generation);
int logger_close(Logger * logger);

#endif
