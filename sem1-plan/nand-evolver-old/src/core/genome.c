/*
 * core/genome.c
 * Create, copy and validate genomes, and the wire layout they are addressed
 * through. The only code that knows where the constant, input, ready, output
 * and internal regions sit.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "genome.h"

#include <stdlib.h>

Genome * genome_create(size_t num_inputs, size_t num_outputs, size_t num_wires)
{
    (void)num_inputs; (void)num_outputs; (void)num_wires;
    abort();    /* stub */
}

Genome * genome_copy(const Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

void genome_free(Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

int genome_validate(const Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

long genome_add_nand(Genome * genome, size_t input1, size_t input2, size_t output)
{
    (void)genome; (void)input1; (void)input2; (void)output;
    abort();    /* stub */
}

int genome_remove_nand(Genome * genome, size_t index)
{
    (void)genome; (void)index;
    abort();    /* stub */
}

size_t genome_constant_wire(const Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

size_t genome_input_wire(const Genome * genome, size_t input)
{
    (void)genome; (void)input;
    abort();    /* stub */
}

size_t genome_ready_wire(const Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

size_t genome_output_wire(const Genome * genome, size_t output)
{
    (void)genome; (void)output;
    abort();    /* stub */
}
