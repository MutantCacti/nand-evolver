/*
 * train/exporter.c
 * Exporter: canonicalise the best genome and write the model file.
 *
 * Called once by the Evolver, at the end of a run, so data leaves a run in one
 * direction. Canonicalisation reorders Nands so that each one's output index
 * is its own index, which is what lets the model file store only (a, b) and
 * what lets one Genome type serve both programs.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

#if defined(TRAINING_EXPORTER_CANONICAL)

int exporter_export(const Genome * best, const char * out_dir)
{
    (void)best; (void)out_dir;
    abort();    /* stub */
}

#else
#error "training.exporter names no Exporter this build provides"
#endif
