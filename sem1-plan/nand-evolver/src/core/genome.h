/*
 * core/genome.h
 * Genome and Nand, in both the training and canonical forms, and the wire
 * layout they are addressed through.
 *
 * Wire layout, as the README defines it:
 *
 *      0                   constant 0
 *      [1, 1+i)            input region     (reserved: read-only to the genome)
 *      1+i                 ready
 *      [2+i, 2+i+m)        output region
 *      [2+i+m, n)          internal
 *
 * A training Nand is three indices. A canonical Nand drops its output index,
 * which its position in the array implies. genome.c is the only code that knows
 * this layout; every other component asks for wire indices by name.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_GENOME_H_
#define CORE_GENOME_H_

#include "word.h"

#include <stddef.h>

/* A Nand in training form: two inputs and an explicit output. */
typedef struct
{
    size_t input1;
    size_t input2;
    size_t output;
}
Nand;

/* A Nand in canonical form: its output index is its position in the array. */
typedef struct
{
    size_t input1;
    size_t input2;
}
CanonicalNand;

typedef struct
{
    Nand * nands;
    size_t num_nands, max_nands;

    size_t num_inputs;      /* i */
    size_t num_outputs;     /* m */
    size_t num_wires;       /* n */
}
Genome;

/* Allocate an empty genome over a wire layout of i inputs and m outputs. */
Genome * genome_create(size_t num_inputs, size_t num_outputs, size_t num_wires);

/* Deep copy, including the Nand array. */
Genome * genome_copy(const Genome * genome);

void genome_free(Genome * genome);

/* 0 if every Nand obeys the layout: inputs in [0, n), outputs in [1+i, n). */
int genome_validate(const Genome * genome);

/* Append a Nand, growing the array. Returns its index, or -1 on failure. */
long genome_add_nand(Genome * genome, size_t input1, size_t input2, size_t output);

/* Remove a Nand, preserving the order of the rest (order is seniority). */
int genome_remove_nand(Genome * genome, size_t index);

/* Wire indices by name. The only sanctioned way to address the layout. */
size_t genome_constant_wire(const Genome * genome);
size_t genome_input_wire(const Genome * genome, size_t input);
size_t genome_ready_wire(const Genome * genome);
size_t genome_output_wire(const Genome * genome, size_t output);

#endif
