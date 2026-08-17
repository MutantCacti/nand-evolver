/*
 * model->c
 * Stateful graph of Nands
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#include "model.h"


Nand *nand_init(size_t input1_index, size_t input2_index, size_t output_index)
{
    Nand *nand = malloc(sizeof(Nand));
    if (!nand) {
        fprintf(stderr, "nand_init: Nand struct allocation failed\n");
    }

    nand->input1_index = input1_index;
    nand->input2_index = input2_index;
    nand->output_index = output_index;

    return nand;
}


Model *model_init(size_t arena_size)
{
    Nand *nands = malloc(sizeof(Nand));
    if (!nands) {
        fprintf(stderr, "model_init: Nand allocation failed\n");
        return NULL;
    }

    int *arena = malloc(sizeof(int) * arena_size);
    if (!arena) {
        fprintf(stderr, "model_init: Arena allocation failed\n");
        return NULL;
    }

    Model *model = malloc(sizeof(Model));
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


void model_add_nand(Model *model, Nand *nand)
{
    if (model->num_nands == model->nands_size)
    {
        // Dynamic resize
        size_t new_nands_size = model->nands_size * 2; // 2^ growth

        Nand* new_nands = realloc(model->nands, sizeof(Nand) * new_nands_size);
        if (!new_nands) {
            fprintf(stderr, "model_add_nand: new_nands allocation failed\n");
        }

        model->nands = new_nands;
        model->nands_size = new_nands_size;
    }

    model->nands[model->num_nands++] = *nand;
}


void model_remove_nand(Model *model, size_t nand_index)
{
    if (nand_index >= model->num_nands) {
        fprintf(stderr, "model_remove_nand: Nand at index %zu does not exist (num_nands=%zu)\n", nand_index, model->num_nands);
        return;
    }

    size_t last_index = --model->num_nands;
    model->nands[nand_index] = model->nands[last_index];
}


void model_update_arena_size(Model *model, size_t new_arena_size)
{
    int *new_arena = realloc(model->arena, sizeof(int) * new_arena_size);
    if (!new_arena) {
        fprintf(stderr, "model_update_arena_size: Arena reallocation failed\n");
        return;
    }

    model->arena = new_arena;
    model->arena_size = new_arena_size;
}


void model_compute_ticks(Model *model, int num_ticks)
{
    for (int i = 0; i < num_ticks; i++) // For each tick
    {
        for (size_t n = 0; n < model->num_nands; n++) // For each Nand
        {
            const Nand *nand = &model->nands[n];
            const int input1_value = model->arena[nand->input1_index];
            const int input2_value = model->arena[nand->input2_index];
            const int output_value = (!input1_value && !input2_value);
            model->arena[nand->output_index] = output_value;
        }
    }
}


void model_free(Model *model)
{
    if (!model) return;
    if (model->nands) free(model->nands);
    free(model);
}
