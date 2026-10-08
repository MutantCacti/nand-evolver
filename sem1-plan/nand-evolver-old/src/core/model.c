/*
 * core/model.c
 * The model file: written by the Exporter at the end of a run, carried compiled
 * in by infer. Loaded from a byte span so a path is never required.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "model.h"

#include <stdlib.h>

int model_write(const char * path, const Model * model)
{
    (void)path; (void)model;
    abort();    /* stub */
}

int model_load(Model * model, const void * bytes, size_t size)
{
    (void)model; (void)bytes; (void)size;
    abort();    /* stub */
}

void model_free(Model * model)
{
    (void)model;
    abort();    /* stub */
}
