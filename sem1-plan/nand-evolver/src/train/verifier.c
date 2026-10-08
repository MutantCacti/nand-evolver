/*
 * train/verifier.c
 * Verifier: the output region against the expected bits, bitwise.
 *
 * Bitwise because expected outputs are bits, so `produced ^ expected` is both
 * the error and the evidence for which wires were wrong. It exists only in
 * train: turning an answer into an error is what selection needs and what the
 * product has no use for.
 *
 * The limit, for a later task: on a number written in binary, a wrong high
 * wire and a wrong low wire count the same, so numeric targets need an error
 * that is not Hamming distance. None of XOR, MUX or one-hot MNIST is one.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

#if defined(TRAINING_VERIFIER_HAMMING)

void verifier_verify(const word * produced, const uint8_t * expected,
                     uint32_t num_outputs, word active,
                     uint32_t * error, word * wrong)
{
    (void)produced; (void)expected; (void)num_outputs;
    (void)active; (void)error; (void)wrong;
    abort();    /* stub */
}

#else
#error "training.verifier names no Verifier this build provides"
#endif
