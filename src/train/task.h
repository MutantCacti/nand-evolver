/*
 * train/task.h
 * A Task is a labelled dataset with a batching protocol.
 *
 * The dataset is stored unpacked (one byte per bit) and is transposed into
 * bit lanes on demand. A Batch holds up to BATCH_LANES examples:
 *
 *      inputs[k]   bit L = input k of the example in lane L
 *      expected[k] bit L = expected output k of the example in lane L
 *
 * Lane L is only an example if bit L of active_lanes is set.
 * A dataset with examples indivisible by BATCH_LANES leaves the final
 * batch incomplete.
 *
 * A Task produces input words indexed 0..num_inputs-1.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#ifndef TASK_H_
#define TASK_H_


#include "../core/word.h"


#include <stddef.h>
#include <stdint.h>


// One example per bit of a word
#define BATCH_LANES WORD_BITS


// A lane-packed set of examples, shared by every genome in a generation
typedef struct
{
    word * inputs;          // num_inputs words
    word * expected;        // num_outputs words
    word   active_lanes;    // bit set ? lane carries an example
    size_t num_inputs, num_outputs;
    size_t num_active_lanes;
}
Batch;


/* A labelled dataset in unpacked form traversable in epochs.
 * A batch is a pure function of (seed, epoch, cursor); logging
 * these values is enough to resume a run exactly (see task_seek). */
typedef struct
{
    const char * name;
    size_t num_examples;
    size_t num_inputs, num_outputs;

    uint8_t * input_bits;   // num_examples * num_inputs
    uint8_t * output_bits;  // num_examples * num_outputs

    uint64_t seed;      // base seed; per-epoch streams derive from it
    size_t epoch;       // shuffles performed
    size_t cursor;      // next example within order[]
    size_t * order;     // this epoch's permutation
}
Task;


// Initialise a named Task
Task * task_init(const char * name, uint64_t seed);


// Free a Task's allocated memory
void task_free(Task * task);


// Allocate a Batch shaped to a Task
Batch * batch_init(const Task * task);


// Free a Batch's allocated memory
void batch_free(Batch * batch);


/* Fill batch with the next up-to-BATCH_LANES examples of the current epoch,
 * advancing to the next epoch when the current one is finished.
 * The final batch of an epoch is short whenever BATCH_LANES does not divide
 * num_examples; batch->num_active_lanes contains the true size of the batch.
 * Returns 0 on success or -1 if the batch is not correctly shaped for the task. */
int task_sample_batch(Task * task, Batch * batch);


// Batches per epoch, i.e. ceil(num_examples / BATCH_LANES)
size_t task_batches_per_epoch(const Task * task);


/* Reposition the walk, rebuilding the epoch's permutation.
 * Restoring a recorded (epoch, cursor) reproduces the batch sequence exactly. */
void task_seek(Task * task, size_t epoch, size_t cursor);


/* Count active lanes whose outputs differ from expected.
 * true_outputs must be words[num_outputs], lane-packed same as the batch.
 * A lane is wrong if any of its output bits differ.
 * Returns 0 and writes *out_errors on success, or -1 on a NULL argument. */
int task_count_errors(const Task * task, const Batch * batch,
                      const word * true_outputs, uint32_t * out_errors);


#endif
