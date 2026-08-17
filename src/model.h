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
    int *arena;
    size_t arena_size;
    size_t num_nands;
    size_t nands_size;
}
Model;


Nand *nand_init(size_t input1_index, size_t input2_index, size_t output_index);
Model *model_init(size_t arena_size);
void model_add_nand(Model *model, Nand *nand);
void model_remove_nand(Model *model, size_t nand_index);
void model_update_arena_size(Model *model, size_t new_arena_size);
void model_compute_ticks(Model *model, int num_ticks);
void model_free(Model *model);


#endif
