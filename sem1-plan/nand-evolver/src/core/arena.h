/*
 * core/arena.h
 * Arena (Example, Tick): the memory space of one individual.
 *
 * Cleared at the start of each example and kept across its rounds, which is how
 * a genome remembers earlier rounds. There is no arena.c: an Arena is a plain
 * span of words, allocated by whoever owns the level that creates it (the
 * Evolver per individual in train, the Harness once at start-up when deployed).
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_ARENA_H_
#define CORE_ARENA_H_

#include "word.h"

typedef struct
{
    word * words;       /* one word per wire (train) or packed wires (infer) */
    size_t num_words;
}
Arena;

#endif
