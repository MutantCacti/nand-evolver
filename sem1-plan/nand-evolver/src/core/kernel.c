/*
 * core/kernel.c
 * The Nand scheduling scheme.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "kernel.h"

#include <stdlib.h>

/* Every live Nand, every tick, writes applied in reverse index order. This is
 * the only scheme P1 has, so it is not configured: an alternative one arrives
 * with the key that selects it. */
void kernel_round(const Genome * genome, Arena * arena,
                  uint32_t tick_limit, uint32_t * ticks)
{
    (void)genome; (void)arena; (void)tick_limit; (void)ticks;
    abort();    /* stub */
}

