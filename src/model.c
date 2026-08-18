/*
 * model.c
 * See definitions in model.h
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#include "model.h"


// MARK: Helpers

size_t num_words(size_t num_bits)
{
    return (num_bits + (WORD_SIZE_BITS - 1)) / WORD_SIZE_BITS;
}


size_t word_index(size_t bit_index)
{
    return bit_index / WORD_SIZE_BITS;
}


size_t word_offset(size_t bit_index)
{
    return bit_index % WORD_SIZE_BITS;
}


// MARK: model_init

Model *model_init(size_t arena_size)
{
    if (arena_size < 1) {
        fprintf(stderr, "model_init: arena_size %zu must be at least 1\n", arena_size);
        return NULL;
    }
    if (arena_size > MAX_ARENA_SIZE) {
        fprintf(stderr, "model_init: arena_size %zu exceeds MAX_ARENA_SIZE %zu\n", arena_size, MAX_ARENA_SIZE);
        return NULL;
    }

    Nand *nands = calloc(1, sizeof(Nand));
    if (!nands) {
        fprintf(stderr, "model_init: Nands allocation failed\n");
        return NULL;
    }

    // The arena is an array of words
    word *arena = calloc(num_words(arena_size), sizeof(word));
    if (!arena) {
        fprintf(stderr, "model_init: Arena allocation failed\n");
        return NULL;
    }

    Model *model = calloc(1, sizeof(Model));
    if (!model) {
        fprintf(stderr, "model_init: Model struct allocation failed\n");
        return NULL;
    }

    model->nands = nands;
    model->arena = arena;
    model->arena_size = arena_size;
    model->num_nands  = 0;
    model->nands_size = 1;

    return model;
}


// MARK: model_add_nand

void model_add_nand(Model *model, size_t input1_index, size_t input2_index, size_t output_index)
{
    if (model->num_nands == model->nands_size)
    {
        // Dynamic resize
        size_t new_nands_size = model->nands_size * 2; // 2^ growth

        Nand* new_nands = realloc(model->nands, sizeof(Nand) * new_nands_size);
        if (!new_nands) {
            fprintf(stderr, "model_add_nand: new_nands allocation failed\n");
            return;
        }

        model->nands = new_nands;
        model->nands_size = new_nands_size;
    }

    // Indices outside the arena are wrapped around using modulo
    // i.e. arena_size = 4, input1_index = 5 -> input1_index = 1
    Nand nand = {
        .input1_index = (input1_index % model->arena_size),
        .input2_index = (input2_index % model->arena_size),
        .output_index = (output_index % model->arena_size)
    };
    model->nands[model->num_nands++] = nand;
}


void model_remove_nand(Model *model, size_t nand_index)
{
    if (nand_index >= model->num_nands) {
        fprintf(stderr, "model_remove_nand: Nand at index %zu out of range (num_nands=%zu)\n", nand_index, model->num_nands);
        return;
    }

    // Remove the Nand while preserving order
    for (size_t i = nand_index; i < model->num_nands - 1; i++) {
        model->nands[i] = model->nands[i + 1];
    }
    model->num_nands--;
}


// MARK: model_update_arena_size

void model_update_arena_size(Model *model, size_t new_arena_size)
{
    if (new_arena_size == model->arena_size) return;
    if (new_arena_size > MAX_ARENA_SIZE) {
        fprintf(stderr, "model_update_arena_size: new_arena_size %zu exceeds MAX_ARENA_SIZE %zu\n", new_arena_size, MAX_ARENA_SIZE);
        return;
    }

    size_t new_num_words = num_words(new_arena_size);
    word *new_arena = realloc(model->arena, new_num_words * sizeof(word));
    if (!new_arena) {
        fprintf(stderr, "model_update_arena_size: Arena reallocation failed\n");
        return;
    }
    if (new_arena_size > model->arena_size)
    {
        // Zero-initialise the newly allocated part of the arena
        size_t old_num_words = num_words(model->arena_size);
        memset(new_arena + old_num_words, 0, (new_num_words - old_num_words) * sizeof(word));
        word old_max_word_mask = (word)-1 << word_offset(model->arena_size);
        new_arena[word_index(model->arena_size)] &= old_max_word_mask - 1;
    }

    model->arena = new_arena;
    model->arena_size = new_arena_size;
}


// MARK: model_compute_ticks

void model_compute_ticks(Model *model, int num_ticks)
{
    if (model->num_nands == 0) return;

    for (int i = 0; i < num_ticks; i++) // For each tick
    {
        // Compute Nand outputs to buffer
        size_t num_output_words = num_words(model->num_nands);
        word outputs[num_output_words];
        memset(outputs, 0, num_output_words * sizeof(word));

        for (size_t j = 0; j < model->num_nands; j++)
        {
            Nand *nand = &model->nands[j];

            word input1_word = model->arena[word_index(nand->input1_index)];
            word input2_word = model->arena[word_index(nand->input2_index)];

            word input1_value = (input1_word >> word_offset(nand->input1_index)) & (word)1;
            word input2_value = (input2_word >> word_offset(nand->input2_index)) & (word)1;

            outputs[word_index(j)] |= (word)!(input1_value & input2_value) << word_offset(j);
        }

        // Write outputs to arena in reverse Nand order
        for (int j = model->num_nands - 1; j >= 0; j--)
        {
            Nand *nand = &model->nands[j];

            word output_word = outputs[word_index(j)];
            word output_value = (output_word >> word_offset(j)) & (word)1;

            word output_mask = (word)1 << word_offset(nand->output_index);
            model->arena[word_index(nand->output_index)] &= ~output_mask;
            model->arena[word_index(nand->output_index)] |= output_value << word_offset(nand->output_index);
        }
    }
}


// MARK: model_free

void model_free(Model *model)
{
    if (!model) return;
    if (model->nands) free(model->nands);
    if (model->arena) free(model->arena);
    free(model);
}
