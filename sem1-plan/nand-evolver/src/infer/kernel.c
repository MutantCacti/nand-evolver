/*
 * infer/kernel.c
 * The packed Kernel: the same schemes as the lane Kernel over canonical Nands, one bit per wire. Must agree with it on every example; test_layouts links both and checks that.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/kernel.h"

#include <stdlib.h>

unsigned packed_kernel_round(const Model * model, Arena * arena, unsigned tick_limit)
{
    (void)model; (void)arena; (void)tick_limit;
    abort();    /* stub */
}

void packed_kernel_tick(const Model * model, Arena * arena)
{
    (void)model; (void)arena;
    abort();    /* stub */
}

int packed_kernel_ready(const Model * model, const Arena * arena)
{
    (void)model; (void)arena;
    abort();    /* stub */
}
