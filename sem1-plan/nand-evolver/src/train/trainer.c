/*
 * train/trainer.c
 * Trainer: changes a genome between examples, in individual mode only.
 *
 * The reference build has no Trainer, and this file then contributes no code
 * at all rather than a function that does nothing: a default implementation is
 * something a build can get by accident, and an #error is not.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

#if defined(TRAINING_TRAINER_NONE)

/* No Trainer: compat.h has already refused any build that would call one. */

#elif defined(TRAINING_TRAINER_DELTA_ERROR)

void trainer_train(Genome * genome, const word * wrong, uint32_t num_outputs,
                   uint64_t seed, uint32_t generation, uint32_t example)
{
    (void)genome; (void)wrong; (void)num_outputs;
    (void)seed; (void)generation; (void)example;
    abort();    /* stub */
}

#else
#error "training.trainer names no Trainer this build provides"
#endif
