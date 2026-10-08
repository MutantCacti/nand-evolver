/*
 * train/kernel.c
 * The lane Kernel: the reference scheme, every Nand every tick. Alternative schedules are #if blocks here.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/kernel.h"

#include <stdlib.h>

/* Tick: evaluate every Nand, then write back in reverse index order. */
static void lane_kernel_tick(const Genome * genome, Arena * arena);

/* Which lanes now read as ready. */
static word lane_kernel_ready(const Genome * genome, const Arena * arena);

unsigned lane_kernel_round(const Genome * genome, Arena * arena, unsigned tick_limit)
{
    (void)genome; (void)arena; (void)tick_limit;
    abort();    /* stub */
}

static void lane_kernel_tick(const Genome * genome, Arena * arena)
{
    (void)genome; (void)arena;
    abort();    /* stub */
}

static word lane_kernel_ready(const Genome * genome, const Arena * arena)
{
    (void)genome; (void)arena;
    abort();    /* stub */
}
