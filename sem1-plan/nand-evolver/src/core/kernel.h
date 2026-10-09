/*
 * core/kernel.h
 * Kernel (Round, Tick): ticks until ready or the tick limit, counting them.
 *
 * The only code that touches individual Nands. A tick evaluates every live
 * Nand against the memory space as it stands, then writes the results back in
 * reverse Nand index order, so the lowest-indexed Nand wins a collision and
 * adding a Nand is a neutral mutation. Ready is checked after each tick and
 * never before the first, so every round runs at least one tick and ready's
 * start value alone can never answer.
 *
 * One function. The tick loop is the Round level and the instruction loop is
 * the Tick level, and both belong to this file; alternative scheduling schemes
 * (a Nand that runs every k-th tick, a next-index frontier) are #if blocks
 * here rather than new interfaces.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_KERNEL_H
#define CORE_KERNEL_H

#include "arena.h"
#include "genome.h"

/* Tick the genome until every example in the word reads ready, or the tick
 * limit is reached, and write each example's tick count into `ticks` (one
 * entry at the reference width).
 *
 * An example whose ready wire has answered is frozen rather than stopped: its
 * bit of the done mask holds its wires at the values they had when it
 * answered, so the examples sharing a word cannot disturb each other, and its
 * tick count is the tick at which its bit turned on. At the reference width
 * the mask is all zeroes or all ones and the freeze is trivially the whole
 * round ending. */
void kernel_round(const Genome * genome, Arena * arena,
                  uint32_t tick_limit, uint32_t * ticks);

#endif
