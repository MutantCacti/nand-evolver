/*
 * core/word.h
 * The word type: the unit of addressable state, shared by both layouts.
 *
 * train stores one word per wire, so bit k belongs to example k and one pass
 * over the Nands evaluates WORD_BITS examples at once. infer stores one bit per
 * wire and runs a single example. Nothing else differs.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_WORD_H_
#define CORE_WORD_H_

#include <stddef.h>
#include <stdint.h>

typedef uint64_t word;

#define WORD_BITS ((size_t)(sizeof(word) * 8))

#endif
