/*
 * core/genome.h
 * Genome: a graph of Nands, and the wire layout it is addressed through.
 *
 * One type serves both programs. A **canonical** genome is not a second type:
 * canonicalisation reorders Nands so that a Nand's position in the array gives
 * its output wire, which is a property of the data, not a different shape. The
 * model file stores only (a, b) because `out` is then implied, and
 * driver/build.py expands it back into this form when it compiles a model into
 * infer. So the Harness and the Kernel take `const Genome *` in both programs
 * and there is no genome/model split to name.
 *
 * The implied index is **not** the array index. A Nand can never write the
 * constant wire or the input region, so the first wire it can write is ready:
 *
 *     canonical: nands[k].out == 1 + num_inputs + k
 *
 * which makes the canonical address space 1 + num_inputs + num_nands, the
 * README's `i+1+N`, and fixes num_nands at 1 + num_outputs + num_internal —
 * one Nand per writable wire, the inert ones included.
 *
 * The wire layout, in order, as the README defines it:
 *
 *     0                                    the constant 0
 *     1 .. num_inputs                      the input region, read-only to the genome
 *     1 + num_inputs                       ready
 *     2 + num_inputs .. + num_outputs      the output region
 *     then num_internal                    the internal region
 *
 * There are no accessor functions for it. core/harness.c is the only code
 * permitted to address the layout, so accessors would exist for a caller that
 * is not allowed to exist; everything else refers to wires by the indices the
 * Nands already carry.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_GENOME_H
#define CORE_GENOME_H

#include "word.h"

/* One Nand: out = ~(a & b), evaluated against the memory space as it stands at
 * the start of the tick. Writes happen in reverse index order, so the
 * lowest-indexed Nand wins a collision and adding a Nand is neutral. */
typedef struct
{
    uint32_t a;
    uint32_t b;
    uint32_t out;
}
Nand;

/* The shape a fresh genome is created with. num_inputs and num_outputs come
 * from the Dataset; the other two are parameters of the search. */
typedef struct
{
    uint32_t num_inputs;
    uint32_t num_outputs;
    uint32_t num_internal;
    uint32_t max_nands;
}
GenomeShape;

typedef struct
{
    Nand * nands;
    uint32_t num_nands;         /* live Nands, evaluated every tick */
    uint32_t max_nands;         /* capacity; the inert reserve is the gap */
    uint32_t num_inputs;
    uint32_t num_outputs;
    uint32_t num_internal;
}
Genome;

/* Total wires, so a caller can size an Arena without knowing the layout. */
#define GENOME_NUM_WIRES(g) \
    ((size_t)2 + (g)->num_inputs + (g)->num_outputs + (g)->num_internal)

/* A fresh genome of this shape: no live Nands, full capacity reserved. */
Genome * genome_create(const GenomeShape * shape);

/* A copy of one, the Mutator's first step. Separate from genome_create so
 * that neither function takes an argument the other ignores. */
Genome * genome_copy(const Genome * parent);

/* 0 when every index is in range, num_nands <= max_nands, and no Nand writes
 * to the constant wire or into the input region. Used after mutation and by
 * the tests; it is the file's reason to exist beyond holding a struct. */
int genome_validate(const Genome * genome);

void genome_free(Genome * genome);

#endif
