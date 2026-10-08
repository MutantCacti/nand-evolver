/*
 * core/harness.h
 * Harness (Individual/Deployment, Example): examples to rounds, driving the
 * protocol the README defines.
 *
 * Per round it writes the input region and ready's start value, has the Kernel
 * run the genome, and reads the output region. It is the same component in both
 * programs: in train it is fed from the Dataset and reaches the Verifier and
 * Trainer only through hooks, so tests can stub them; deployed it is fed input
 * records by whoever embeds the model and calls neither.
 *
 * lane_* (train/harness.c) packs WORD_BITS examples into the lanes of each
 * word. packed_* (infer/harness.c) runs one example, one bit per wire.
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

/* One exchange of an example: the input bits to write, and, when graded, the
 * output bits expected back. Bits are raw: no encoding is invented anywhere. */
typedef struct
{
    const word * inputs;        /* num_inputs bits, one example's worth */
    const word * expected;      /* num_outputs bits; NULL when not graded */
    int graded;
}
Round;

/* One problem: a sequence of rounds. XOR is one; an MNIST image fed a row at a
 * time is 28, of which only the last is graded. */
typedef struct
{
    const Round * rounds;
    size_t num_rounds;
}
Example;

/* What one example's measurement yields. Travels up unchanged: nothing below
 * the Selector combines these. */
typedef struct
{
    uint32_t error;     /* wrong output bits over the example's graded rounds */
    uint32_t ticks;     /* ticks used over all of its rounds */
}
Record;

/* The lane Harness's only route to train-only components. */
typedef struct
{
    /* Verifier, on a graded round: produced vs expected output wires. Writes
     * the per-wire wrong mask as evidence, and returns the error it adds. */
    uint32_t (*verify)(void * context,
                       const word * produced, const word * expected,
                       size_t num_outputs, word active_lanes, word * wrong_out);

    /* Trainer, between examples: change the genome from results so far. */
    int (*train)(void * context, Genome * genome,
                 const Record * records, size_t count);

    void * context;     /* NULL hooks mean the component is not configured in */
}
HarnessHooks;

/* Individual: measure one genome over the examples it was handed, filling one
 * Record per example. The caller decides how examples were divided. */
int lane_harness_individual(const Genome * genome, Arena * arena,
                            const Example * examples, size_t num_examples,
                            const HarnessHooks * hooks, unsigned tick_limit,
                            Record * records_out);

/* Example: one lane group of up to WORD_BITS examples, packed into lanes. */
int lane_harness_example(const Genome * genome, Arena * arena,
                         const Example * group, size_t group_size,
                         const HarnessHooks * hooks, unsigned tick_limit,
                         Record * records_out);

/* Where a deployed model's examples come from and where its answers go. Keeps
 * core free of any opinion about stdin and stdout. */
typedef struct
{
    /* Fill one example's input bits. 0 on success, -1 at end of stream. */
    int (*next)(void * context, word * inputs, size_t num_inputs);
    int (*emit)(void * context, const word * outputs, size_t num_outputs);
    void * context;
}
DeploymentIO;

/* Deployment: one model running for as long as it is switched on. One input
 * record in, one output record out, lock-step. The model always answers. */
int packed_harness_deployment(const Model * model, Arena * arena,
                              const DeploymentIO * io, unsigned tick_limit);

/* Example: one example's rounds, one bit per wire. */
int packed_harness_example(const Model * model, Arena * arena,
                           const Example * example, unsigned tick_limit,
                           word * outputs_out);

#endif
