/*
 * core/arena.h
 * Arena: the memory space of one genome under measurement.
 *
 * A type and nothing else. Memory belongs to one level and is used below it,
 * and the level above the Harness owns this one: in train the Evolver creates
 * it per work unit and reuses it across that unit's examples, and deployed
 * main creates it once at start-up. Neither needs a function to do that, and
 * giving the Arena one would invite a third owner.
 *
 * The Harness resets it at every example boundary (harness_reset) and the
 * Kernel writes it once per tick. Nothing else touches it.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_ARENA_H
#define CORE_ARENA_H

#include "word.h"

typedef struct
{
    word * wires;
    size_t num_wires;
}
Arena;

#endif
