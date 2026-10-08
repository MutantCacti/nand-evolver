/*
 * train/verifier.c
 * Verifier: produced against expected output wires on a graded round. Bitwise, so it reports which wires are wrong as evidence. Reached by the Harness through a hook; exists only in train.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

uint32_t verifier_verify(const word * produced, const word * expected,
                         size_t num_outputs, word active_lanes,
                         word * wrong_out)
{
    (void)produced; (void)expected;
    (void)num_outputs; (void)active_lanes; (void)wrong_out;
    abort();    /* stub */
}
