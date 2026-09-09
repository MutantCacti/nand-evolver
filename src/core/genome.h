/*
 * core/genome.h
 * Nand and Genome struct definitions.
 * A genome is a graph of Nands and their communication protocol.
 * This is encoded as a structured address space of wires:
 *
 *  i: number of (reserved) input wires
 *  m: number of output wires
 *  n: total address space
 *
 *  wire(s)         description
 *  ---             ---
 *  0               const 0         // constant reference
 *  [1, i+1)        input space     // written at tick 0
 *  i+1             ready wire      // active low
 *  [i+2, m+i+2)    output space    // read when ready
 *  [m+i+2, n)      internal space  // non-output working memory
 *
 * e.g. 0 01 0 1 010110
 *      ^ ^  ^ ^ ^
 *  const |  | | internal
 *   inputs  | output
 *        ready
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#ifndef GENOME_H_
#define GENOME_H_


#include <stddef.h>
#include <stdint.h>


// A Nand gate over some addressable memory
typedef struct
{
    size_t input1_index;
    size_t input2_index;
    size_t output_index;
}
Nand;


// A dynamic array of Nand gates
typedef struct
{
    // Graph
    Nand * nands;
    size_t num_nands, max_nands;

    // Protocol
    size_t address_space;
    size_t num_inputs, num_outputs;
    size_t * output_indices;

    // Limit
    size_t max_ticks;
}
Genome;


#endif
