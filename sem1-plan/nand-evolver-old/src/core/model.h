/*
 * core/model.h
 * The model file: a training run's only output, and the deployed program's only
 * input.
 *
 * Holds the canonical genome, its input and output sizes, and room for the
 * memory state it should start from (a child beginning from its parent's
 * memory). The Exporter writes it; infer carries it compiled in, so it is
 * loaded from a byte span rather than a path.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_MODEL_H_
#define CORE_MODEL_H_

#include "genome.h"
#include "word.h"

#include <stddef.h>

typedef struct
{
    CanonicalNand * nands;
    size_t num_nands;

    size_t num_inputs;
    size_t num_outputs;
    size_t num_wires;

    const word * initial_state;     /* NULL when the arena starts cleared */
    size_t num_initial_words;
}
Model;

/* Serialise a canonical genome to path. Called only by the Exporter. */
int model_write(const char * path, const Model * model);

/* Read a model from a byte span: a mapped file, or infer's compiled-in blob. */
int model_load(Model * model, const void * bytes, size_t size);

void model_free(Model * model);

#endif
