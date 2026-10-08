/*
 * infer/harness.c
 * The packed Harness: one example, one bit per wire. Drives the same protocol as the lane Harness and calls neither a Verifier nor a Trainer, because the product has neither.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/harness.h"
#include "../core/kernel.h"

#include <stdlib.h>

int packed_harness_deployment(const Model * model, Arena * arena,
                              const DeploymentIO * io, unsigned tick_limit)
{
    (void)model; (void)arena; (void)io; (void)tick_limit;
    abort();    /* stub */
}

int packed_harness_example(const Model * model, Arena * arena,
                           const Example * example, unsigned tick_limit,
                           word * outputs_out)
{
    (void)model; (void)arena; (void)example; (void)tick_limit; (void)outputs_out;
    abort();    /* stub */
}
