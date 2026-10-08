/*
 * train/trainer.c
 * Trainer: changes a genome between examples, from the results so far. Individual mode only, and reached by the Harness through a hook.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

#if defined(TRAINING_TRAINER_NONE)

/* No Trainer in this build: nothing changes a genome during its own
 * measurement, so this file contributes no code. The lane Harness compiles
 * without an example-boundary callback at all. */
typedef int trainer_not_built; /* C requires a translation unit to declare something */

#elif defined(TRAINING_TRAINER_DELTA_ERROR)

int trainer_train(void * context, Genome * genome,
                  const Record * records, size_t count)
{
    (void)context; (void)genome; (void)records; (void)count;
    abort();    /* stub */
}

#else
#error "training.trainer names no Trainer this build provides"
#endif
