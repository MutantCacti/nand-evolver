/*
 * model.h
 * Model struct and function definition
 * A Model is a stateful graph of Nands
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#ifndef MODEL_H_
#define MODEL_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


typedef uint64_t word;
#define WORD_SIZE_BITS (size_t)(sizeof(word) * 8)
#define MAX_ARENA_SIZE (size_t)(8388608) // 1 MiB


typedef struct Nand
{
    size_t input1_index;
    size_t input2_index;
    size_t output_index;
}
Nand;


typedef struct Model
{
    Nand *nands;
    word *arena;
    size_t arena_size;
    size_t num_nands;
    size_t nands_size;
}
Model;


// Helpers
size_t num_words(size_t num_bits);
size_t word_index(size_t bit_index);
size_t word_offset(size_t bit_index);


// Model interface
Model *model_init(size_t arena_size);
void model_add_nand(Model *model, size_t input1_index, size_t input2_index, size_t output_index);
void model_remove_nand(Model *model, size_t nand_index);
void model_update_arena_size(Model *model, size_t new_arena_size);
void model_compute_ticks(Model *model, int num_ticks);
void model_free(Model *model);


#endif
