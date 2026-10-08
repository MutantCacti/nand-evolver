/*
 * core/kernel.c
 * The Nand scheduling scheme.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "kernel.h"

#include <stdlib.h>

#if defined(PROTOCOL_KERNEL_REFERENCE)

/* The reference: every live Nand, every tick, writes applied in reverse index
 * order. Alternative schemes are #elif branches beside this one. */
void kernel_round(const Genome * genome, Arena * arena,
                  uint32_t tick_limit, uint32_t * ticks)
{
    (void)genome; (void)arena; (void)tick_limit; (void)ticks;
    abort();    /* stub */
}

#else
#error "protocol.kernel names no Kernel this build provides"
#endif
