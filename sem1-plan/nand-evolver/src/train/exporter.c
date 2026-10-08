/*
 * train/exporter.c
 * Exporter: canonicalises the best genome the run found, in the configured way, and writes the model file through core/model.c. Called once by the Evolver.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int exporter_export(const Config * config, const Genome * best,
                    const word * initial_state, size_t num_initial_words,
                    const char * path)
{
    (void)config; (void)best; (void)initial_state;
    (void)num_initial_words; (void)path;
    abort();    /* stub */
}
