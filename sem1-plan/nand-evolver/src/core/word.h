/*
 * core/word.h
 * The word: one per wire, every bit of it holding the same value.
 *
 * A wire is all zeroes or all ones, never a mixture, so a Nand is ~(a & b)
 * whatever the width of a word. That is what lets one Kernel serve both the
 * reference and the lane layout that comes later, and it is an invariant the
 * whole design rests on: write a 1 into a wire instead of WORD_ALL and the
 * reference still looks correct in bit 0 while lanes break for a reason
 * nobody connects to the write that caused it. test_protocol asserts it.
 *
 * The general rule, which the reference and the optimisation share:
 *
 *     bit j of a wire's word holds the value of example (j mod lane_width).
 *
 * EXECUTION_LANE_WIDTH is 1 here, so every bit of a word is the same example.
 * In P2 it becomes WORD_BITS and one pass over the Nands evaluates that many
 * examples; only what the Harness writes into the words changes.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_WORD_H
#define CORE_WORD_H

#include "compat.h"

#include <stddef.h>
#include <stdint.h>

#if EXECUTION_WORD_BITS == 8
typedef uint8_t word;
#elif EXECUTION_WORD_BITS == 16
typedef uint16_t word;
#elif EXECUTION_WORD_BITS == 32
typedef uint32_t word;
#elif EXECUTION_WORD_BITS == 64
typedef uint64_t word;
#else
#error "execution.word_bits names no word this build provides"
#endif

#define WORD_BITS ((size_t)(sizeof(word) * 8))
#define WORD_ALL  ((word)~(word)0)

/* A wire's value as a word: every bit set, or none. */
#define WORD_OF(bit) ((bit) ? WORD_ALL : (word)0)

#endif
