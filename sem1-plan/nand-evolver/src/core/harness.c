/*
 * core/harness.c
 * The protocol in one place, for both programs.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "harness.h"
#include "kernel.h"

#include <stdlib.h>

#if !defined(PROTOCOL_READY_START) || !defined(PROTOCOL_READY_VALUE)
#error "protocol.ready_start and protocol.ready_value must both be configured"
#endif

void harness_reset(const Genome * genome, Arena * arena, const uint8_t * initial)
{
    (void)genome; (void)arena; (void)initial;
    abort();    /* stub */
}

void harness_round(const Genome * genome, Arena * arena,
                   const uint8_t * inputs, uint8_t * outputs,
                   uint32_t tick_limit, uint32_t * ticks)
{
    (void)genome; (void)arena; (void)inputs; (void)outputs;
    (void)tick_limit; (void)ticks;
    abort();    /* stub */
}
