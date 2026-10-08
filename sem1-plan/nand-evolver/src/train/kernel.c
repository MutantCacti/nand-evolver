/*
 * train/kernel.c
 * The lane Kernel: the reference scheme, every Nand every tick. Alternative schedules are #if blocks here.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/kernel.h"

#include <stdlib.h>

#if defined(PROTOCOL_KERNEL_REFERENCE)

/* The reference scheme: every Nand, every tick. An alternative scheme (a Nand
 * that runs only every k-th tick, or one driven by which wires changed) adds an
 * #elif branch here. There is no default: a build that names no Kernel fails to
 * compile rather than quietly getting this one. */

/* Tick: evaluate every Nand, then write back in reverse index order. */
static void lane_kernel_tick(const Genome * genome, Arena * arena);

/* Which lanes now read as ready. */
static word lane_kernel_ready(const Genome * genome, const Arena * arena);

uint32_t lane_kernel_round(const Genome * genome, Arena * arena, uint32_t tick_limit)
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

#else
#error "protocol.kernel names no Kernel this build provides"
#endif
