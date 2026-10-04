/*
 * core/word.h
 * The unit of addressable state.
 *
 * A word is one machine word of bits.
 * There are two distinct word semantics used:
 *
 *      1. A word is wires; an arena is a contiguous block of
 *         bits indexed by (word and then bit offset within word).
 *         run/ does this to optimise memory usage for inference.
 *
 *      2. A word is examples; an arena is an array of wires
 *         where each word represents 64 different states of that
 *         same wire across 64 examples in the dataset (a batch).
 *         train/ does this to speed up training with SIMD.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#ifndef WORD_H_
#define WORD_H_


#include <stddef.h>
#include <stdint.h>


typedef uint64_t word;


#define WORD_BITS ((size_t)(sizeof(word) * 8))


#endif
