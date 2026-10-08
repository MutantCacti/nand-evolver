/*
 * core/harness.h
 * Harness: the README protocol, and the only code that knows the wire layout.
 *
 * It owns no loop. It is called with arguments and returns a value, so it
 * never knows where its inputs come from or where its outputs go: in train the
 * Evolver calls it from the Example level, and deployed main calls it from the
 * record loop. That is what makes a trained genome mean the same thing when
 * deployed — both programs walk the same path through the same code.
 *
 * Two functions, because they are two operations rather than two levels. On a
 * reset record infer resets and runs nothing; on an input record it runs a
 * round and resets nothing. Folding them would mean a flag that selects which
 * of two things to do, which is a worse interface than two names.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_HARNESS_H
#define CORE_HARNESS_H

#include "arena.h"
#include "genome.h"

/* Put the Arena into its start-of-example state: every wire 0, the constant
 * wire 0, and the internal region set from `initial` when it is given.
 *
 * `initial` is packed bits over the internal region, or NULL for an empty
 * start. Its source differs by program — the model file deployed, a parent's
 * memory in train — which is why it is an argument and not something read
 * here. One example's worth of bits is broadcast across every lane of a word,
 * so a reset is the same call whatever the lane width.
 *
 * An example is a lifetime: this is called at its start and never between the
 * rounds inside it. */
void harness_reset(const Genome * genome, Arena * arena, const uint8_t * initial);

/* Run one round: write `inputs` into the input region and ready's start value
 * into the ready wire, call the Kernel, and return the output region.
 *
 * `inputs` is packed bits, bit k being input wire k, each expanded to a whole
 * word on the way in.
 *
 * The return is a pointer to the first of `genome->num_outputs` words inside
 * the Arena, one per output wire, valid until the next call. It is returned
 * rather than copied out because a caller handed only an Arena would have to
 * work out where the output region begins, and that is the one piece of
 * knowledge this file exists to keep: the Verifier is given this pointer, and
 * deployed main packs its bits into a record from it. Neither computes an
 * offset.
 *
 * `ticks` receives one tick count per example in the word (one entry at the
 * reference width). Ticks are charged, so they are per example from the start:
 * a group charged its slowest example's ticks would make the number of
 * examples per word change the result.
 *
 * There is no failure. At the tick limit the output region is returned as it
 * stands, which is forced closure: the model always answers. */
const word * harness_round(const Genome * genome, Arena * arena,
                           const uint8_t * inputs,
                           uint32_t tick_limit, uint32_t * ticks);

#endif
