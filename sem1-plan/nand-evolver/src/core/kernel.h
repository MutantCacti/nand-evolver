/*
 * core/kernel.h
 * Kernel (Round, Tick): ticks until ready or the tick limit, counting them.
 *
 * The only code that touches individual Nands. A tick evaluates every Nand
 * against the current memory space, then writes the results back in reverse
 * Nand index order, so the lowest-indexed Nand wins a collision. Ready is
 * checked after each tick and never before the first, so every round runs at
 * least one tick and ready's start value alone can never answer.
 *
 * Two implementations of this one interface: lane_* in train/kernel.c and
 * packed_* in infer/kernel.c, so both link into the differential test.
 * Alternative scheduling schemes are #if blocks inside those files.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_KERNEL_H_
#define CORE_KERNEL_H_

#include "arena.h"
#include "genome.h"
#include "model.h"
#include "word.h"

#include <stddef.h>

/* Round: tick until ready or the limit. Returns ticks used.
 * Lanes still unready at the limit answer with whatever their output holds. */
unsigned lane_kernel_round(const Genome * genome, Arena * arena, unsigned tick_limit);
unsigned packed_kernel_round(const Model * model, Arena * arena, unsigned tick_limit);

/* Tick: evaluate every Nand, then write back in reverse index order. */
void lane_kernel_tick(const Genome * genome, Arena * arena);
void packed_kernel_tick(const Model * model, Arena * arena);

/* Which lanes (train) or whether this example (infer) now reads as ready. */
word lane_kernel_ready(const Genome * genome, const Arena * arena);
int packed_kernel_ready(const Model * model, const Arena * arena);

#endif
