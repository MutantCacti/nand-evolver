/*
 * train/trainer.c
 * Trainer: changes a genome between examples, from the results so far. Individual mode only, and reached by the Harness through a hook.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int trainer_train(void * context, Genome * genome,
                  const Record * records, size_t count)
{
    (void)context; (void)genome; (void)records; (void)count;
    abort();    /* stub */
}
