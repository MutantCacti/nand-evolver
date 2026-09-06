/*
 * train/task.c
 * Dataset storage, batch sampling and error counting.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#include "task.h"
#include "rng.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// MARK: Datasets


// XOR
static const uint8_t XOR_INPUT_BITS[4 * 2] =
{
    0, 0,
    0, 1,
    1, 0,
    1, 1
};

static const uint8_t XOR_OUTPUT_BITS[4 * 1] =
{
    0,
    1,
    1,
    0
};


// MUX
static const uint8_t MUX_INPUT_BITS[8 * 3] =
{
    0, 0, 0,
    0, 0, 1,
    0, 1, 0,
    0, 1, 1,
    1, 0, 0,
    1, 0, 1,
    1, 1, 0,
    1, 1, 1
};

static const uint8_t MUX_OUTPUT_BITS[8 * 1] =
{
    0,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
};


// MARK: Epochs

// Rebuild order[] as this epoch's permutation
static void task_shuffle(Task * task)
{
    for (size_t i = 0; i < task->num_examples; i++) task->order[i] = i;

    /* There's no point shuffling a dataset that fits in one batch
     * because only the lanes will be randomised and popcount doesn't care.
     * Not shuffling improves human readability for small tasks like XOR. */
    if (task->num_examples <= BATCH_LANES) return;

    Rng rng = rng_stream(task->seed, task->epoch);

    for (size_t i = 0; i + 1 < task->num_examples; i++)
    {
        size_t j = i + (size_t)rng_below(&rng, task->num_examples - i);

        size_t swap = task->order[i];
        task->order[i] = task->order[j];
        task->order[j] = swap;
    }
}


// MARK: task_init


static void task_alloc(Task * task, uint64_t seed)
{
    task->seed = seed;
    task->epoch = 0;
    task->cursor = 0;
    task_shuffle(task);
}


static Task * task_from_arrays(const char * name, uint64_t seed,
                               size_t num_examples, size_t num_inputs, size_t num_outputs,
                               const uint8_t * input_bits, const uint8_t * output_bits)
{
    if (num_examples < 1 || num_inputs < 1 || num_outputs < 1)
    {
        fprintf(stderr, "task_from_arrays: %s has an empty dimension\n", name);
        return NULL;
    }

    Task * task = calloc(1, sizeof(Task));
    if (!task)
    {
        perror("task_from_arrays: Failed to calloc task");
        return NULL;
    }

    task->name         = name;
    task->num_examples = num_examples;
    task->num_inputs   = num_inputs;
    task->num_outputs  = num_outputs;

    task->input_bits  = calloc(num_examples * num_inputs,  sizeof(uint8_t));
    task->output_bits = calloc(num_examples * num_outputs, sizeof(uint8_t));
    task->order       = calloc(num_examples, sizeof(size_t));

    if (!task->input_bits || !task->output_bits || !task->order)
    {
        perror("task_from_arrays: Failed to calloc dataset");
        task_free(task);
        return NULL;
    }

    memcpy(task->input_bits,  input_bits,  num_examples * num_inputs);
    memcpy(task->output_bits, output_bits, num_examples * num_outputs);

    task_alloc(task, seed);

    return task;
}


/* TODO: Load a dataset too large to be a C array (MNIST). */
static Task * task_from_file(const char * name, uint64_t seed, const char * path)
{
    (void)seed;

    fprintf(stderr, "task_from_file: '%s' is not implemented (path '%s')\n", name, path);
    return NULL;
}


/* This is a semi cursed implementation of a class registry in C.
 * To add a Task, write it as a C array, include it in this file
 * and key its name to a task_from_arrays call. */
Task * task_init(const char * name, uint64_t seed)
{
    if (!name)
    {
        fprintf(stderr, "task_init: name must not be NULL\n");
        return NULL;
    }

    if (strcmp(name, "xor") == 0)
    {
        return task_from_arrays("xor", seed, 4, 2, 1, XOR_INPUT_BITS, XOR_OUTPUT_BITS);
    }

    if (strcmp(name, "mux") == 0)
    {
        return task_from_arrays("mux", seed, 8, 3, 1, MUX_INPUT_BITS, MUX_OUTPUT_BITS);
    }

    fprintf(stderr, "task_init: Unknown task '%s'\n", name);
    return NULL;
}


// MARK: task_free

void task_free(Task * task)
{
    if (!task) return;
    if (task->input_bits) free(task->input_bits);
    if (task->output_bits) free(task->output_bits);
    if (task->order) free(task->order);
    free(task);
}


// MARK: batch_init

Batch * batch_init(const Task * task)
{
    if (!task)
    {
        fprintf(stderr, "batch_init: task must not be NULL\n");
        return NULL;
    }

    Batch * batch = calloc(1, sizeof(Batch));
    if (!batch)
    {
        perror("batch_init: Failed to calloc batch");
        return NULL;
    }

    batch->inputs = calloc(task->num_inputs,  sizeof(word));
    batch->expected = calloc(task->num_outputs, sizeof(word));

    if (!batch->inputs || !batch->expected)
    {
        perror("batch_init: Failed to calloc batch words");
        batch_free(batch);
        return NULL;
    }

    batch->num_inputs = task->num_inputs;
    batch->num_outputs = task->num_outputs;
    batch->active_lanes = 0;
    batch->num_active_lanes = 0;

    return batch;
}


// MARK: batch_free

void batch_free(Batch * batch)
{
    if (!batch) return;
    if (batch->inputs) free(batch->inputs);
    if (batch->expected) free(batch->expected);
    free(batch);
}


// MARK: task_sample_batch

int task_sample_batch(Task * task, Batch * batch)
{
    if (!task || !batch) return -1;

    if (batch->num_inputs != task->num_inputs || batch->num_outputs != task->num_outputs)
    {
        fprintf(stderr, "task_sample_batch: Batch is shaped %zux%zu, task '%s' is %zux%zu\n",
                batch->num_inputs, batch->num_outputs,
                task->name, task->num_inputs, task->num_outputs);
        return -1;
    }

    // End of an epoch: reshuffle and start fresh
    if (task->cursor >= task->num_examples)
    {
        task->epoch++;
        task->cursor = 0;
        task_shuffle(task);
    }

    // Short final batch when BATCH_LANES does not divide num_examples
    size_t remaining = task->num_examples - task->cursor;
    size_t count = remaining < BATCH_LANES ? remaining : BATCH_LANES;

    memset(batch->inputs, 0, task->num_inputs * sizeof(word));
    memset(batch->expected, 0, task->num_outputs * sizeof(word));
    batch->active_lanes = 0;

    for (size_t lane = 0; lane < count; lane++)
    {
        size_t example = task->order[task->cursor + lane];

        const uint8_t * inputs = task->input_bits + example * task->num_inputs;
        const uint8_t * outputs = task->output_bits + example * task->num_outputs;

        // Shift the example bit into the right lane
        for (size_t i = 0; i < task->num_inputs; i++)
        {
            batch->inputs[i] |= (word)(inputs[i] & 1) << lane;
        }

        for (size_t i = 0; i < task->num_outputs; i++)
        {
            batch->expected[i] |= (word)(outputs[i] & 1) << lane;
        }

        batch->active_lanes |= (word)1 << lane;
    }

    batch->num_active_lanes = count;
    task->cursor += count;

    return 0;
}


// MARK: task_batches_per_epoch

size_t task_batches_per_epoch(const Task * task)
{
    if (!task || task->num_examples == 0) return 0;
    return (task->num_examples + BATCH_LANES - 1) / BATCH_LANES;
}


// MARK: task_seek

void task_seek(Task * task, size_t epoch, size_t cursor)
{
    if (!task) return;

    task->epoch = epoch;
    task->cursor = cursor < task->num_examples ? cursor : task->num_examples;
    task_shuffle(task);
}


// MARK: task_count_errors

int task_count_errors(const Task * task, const Batch * batch,
                      const word * actual_outputs, uint32_t * out_errors)
{
    if (!task || !batch || !actual_outputs || !out_errors) return -1;

    // A lane is wrong if any output bit differs
    word wrong = 0;
    for (size_t i = 0; i < task->num_outputs; i++)
    {
        wrong |= actual_outputs[i] ^ batch->expected[i];
    }

    // Inactive lanes are not evaluated
    wrong &= batch->active_lanes;

    *out_errors = (uint32_t)__builtin_popcountll((unsigned long long)wrong);

    return 0;
}
