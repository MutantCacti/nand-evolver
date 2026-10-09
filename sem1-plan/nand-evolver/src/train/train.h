/*
 * train/train.h
 * The boundary of every train component, for main.c and the tests.
 *
 * One function per component wherever a component can be one function. The
 * exceptions are the two files that own a resource — dataset.c and logger.c,
 * which need an open and a close — and evolver.c, which owns two levels.
 *
 * Configuration does not appear in any signature. Protocol, training,
 * inference and compile-time execution choices are `#define`s; parameters and
 * the thread count are parsed once at start-up into the read-only global
 * below. Configuration that cannot change and is identical everywhere is not
 * a parameter, and passing it would only invite someone to vary it.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef TRAIN_TRAIN_H
#define TRAIN_TRAIN_H

#include "../core/arena.h"
#include "../core/genome.h"
#include "../core/harness.h"
#include "../core/kernel.h"
#include "../core/model.h"

#include <stdint.h>

/* ---------------------------------------------------------------- Config  */

/* The run-time half of the experiment file. Everything here is a number read
 * once and never written again. */
typedef struct
{
    /* parameter.* */
    uint32_t population;
    uint32_t generations;
    uint32_t tick_limit;
    uint32_t initial_nands;     /* live Nands in the first generation */
    uint32_t max_nands;         /* capacity; the inert reserve is the gap */
    uint32_t internal_wires;
    uint32_t mutation_rate;     /* expected mutations per thousand live Nands */

    /* execution.*, read at start-up because varying it must not need a rebuild */
    uint32_t threads;
}
Config;

/* Set once by config_load and const thereafter. */
extern const Config * config;

/* Parse the experiment file into the global. `path` is NULL for the copy
 * embedded in this binary, which is the experiment path and the one that
 * carries the reproducibility guarantee. A given path is refused unless its
 * build hash matches this binary's. 0 on success. */
int config_load(const char * path);

/* --------------------------------------------------------------- Dataset  */

/* The train split, mapped read-only. Written by the Driver, never by C.
 * Every example is the same size, so a lookup is an index.
 *
 * **This header is the format's specification.** The file crosses between the
 * Driver and the C side, so one of them owns the layout in full rather than
 * both describing it; driver/dataset.py writes it and points here.
 *
 *     "NDS1"                      4 bytes
 *     u32 num_inputs              bits written into the input region per round
 *     u32 num_outputs             bits compared on a graded round
 *     u32 rounds                  rounds per example
 *     u32 num_examples
 *     u32 num_graded              how many of `rounds` are graded
 *     ceil(rounds / 8)            graded mask, bit r set when round r is graded
 *     num_examples * {
 *         rounds     * ceil(num_inputs  / 8)   input bits, every round in order
 *         num_graded * ceil(num_outputs / 8)   expected bits, graded rounds only
 *     }
 *
 * Every integer is little-endian and every bit field is packed LSB-first, bit
 * k being wire k of its region — the same packing a record uses. Because every
 * example is the same size, the lookup below is an index rather than a scan. */
typedef struct
{
    const uint8_t * map;
    size_t size;

    uint32_t num_inputs;        /* bits per round, written into the input region */
    uint32_t num_outputs;       /* bits compared on a graded round */
    uint32_t rounds;            /* rounds per example */
    uint32_t num_examples;
    uint32_t num_graded;        /* how many of `rounds` are graded */

    const uint8_t * graded;     /* bitmask over rounds, bit r set when graded */
    size_t example_size;
}
Dataset;

int  dataset_open(Dataset * dataset, const char * path);
void dataset_close(Dataset * dataset);

/* The example at `position` within `generation`. A pure lookup: the same
 * (seed, generation, position) always names the same example, and no order is
 * ever stored or shuffled, so a run gives the same result however its work is
 * divided. Returns the example's input bits, all its rounds in order, and
 * writes its expected output bits for the graded rounds to `expected`. */
const uint8_t * dataset_example(const Dataset * dataset, uint64_t seed,
                                uint32_t generation, uint32_t position,
                                const uint8_t ** expected);

/* ------------------------------------------------------------------- Rng  */

/* A random value from the seed and a position in the trees, e.g.
 * (generation, child, nand). There is no stored state to share or advance, so
 * two threads drawing for different positions cannot interfere and the result
 * does not depend on who drew first. */
uint64_t rng_value(uint64_t seed, const uint32_t * path, size_t depth);

/* --------------------------------------------------------------- Evolver  */

/* One Run: generations of measurement and selection from one seed, ending
 * with the best genome handed to the Exporter. Resumes from a checkpoint in
 * `out_dir` when one is there, and returns 0 only once the model file is
 * complete. The one place work is divided. */
int evolver_run(const Dataset * dataset, uint64_t seed, const char * out_dir);

/* -------------------------------------------------- Components it calls  */

/* Choose `num_parents` genomes. The only component that combines per-example
 * results, which is why it is handed them unreduced: `error` and `ticks` are
 * num_genomes * num_examples, genome-major. Lexicase selection needs every
 * example separately, so nothing above may have summed them.
 *
 * `parents[0]` is the genome this policy ranks first, and **it must survive
 * into the next population unmutated**. That is an invariant every policy
 * obeys, not a policy of its own: it makes the best of the last generation the
 * best of the whole run by induction, so nothing ever compares two
 * generations. It is phrased as a ranking rather than a score because a policy
 * like lexicase never produces one number, and an invariant it could not state
 * would not be an invariant. */
void selector_select(const uint32_t * error, const uint32_t * ticks,
                     uint32_t num_genomes, uint32_t num_examples,
                     uint64_t seed, uint32_t generation,
                     uint32_t * parents, uint32_t num_parents);

/* A child of `parent` with random changes: rewiring an index, or turning on
 * one of the inert Nands held in reserve. Pure in (seed, generation, child),
 * so the population of a generation does not depend on the order it was made
 * in.
 *
 * Never called for the genome the Selector ranked first: that one is copied
 * into the next population unchanged, which is the other half of the invariant
 * above. */
Genome * mutator_mutate(const Genome * parent, uint64_t seed,
                        uint32_t generation, uint32_t child);

/* Compare one graded round's output against the expected bits. Bitwise, so it
 * reports which wires were wrong as evidence the Mutator can use.
 *
 * A leaf: the Evolver calls it after harness_example returns, on round r's
 * outputs at `outputs + r * num_outputs`. Nothing on the path a genome walks
 * knows that grading exists, which is what keeps that path identical in both
 * programs.
 *
 * `produced` is one word per output wire;
 * `expected` is the Dataset's packed bits, expanded here because this is the
 * only place that needs them as words. `active` masks the examples in the word
 * that are still in play. `error` takes one count per example, never one per
 * group: ticks and error are per example from the start, so the number of
 * examples sharing a word can never change a result. */
void verifier_verify(const word * produced, const uint8_t * expected,
                     uint32_t num_outputs, word active,
                     uint32_t * error, word * wrong);

/* Canonicalise the best genome and write the model file, once, at the end of
 * a run. Data leaves a run one way: Evolver -> Exporter -> model file. */
int exporter_export(const Genome * best, const char * out_dir);

/* ------------------------------------------------------------------ Log  */

/* The run log: one JSON object per line, written by whichever component has
 * something to record. Ordered by wall time and never out of generation
 * order — only generations have a real order, and an example's index within
 * one is arbitrary and may be produced in parallel. Per-thread buffers are
 * merged at generation boundaries, which is where the population is at rest.
 *
 * Every count the Driver reads is an integer, so a generation's record is
 * identical however the work was divided. A mean would not be: floating-point
 * addition is not associative, so a mean accumulated across eight threads
 * differs in its last bits from the same mean on one, and the per-generation
 * records are exactly what the determinism test compares. The Driver divides. */
int  logger_open(const char * out_dir);
void logger_event(uint32_t generation, const char * component,
                  const char * event, const char * fmt, ...);
void logger_close(void);

#endif
