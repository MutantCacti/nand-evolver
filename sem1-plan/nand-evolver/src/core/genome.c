/*
 * core/genome.c
 * Genome creation, copying and validation.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "genome.h"

#include <stdlib.h>

Genome * genome_create(const GenomeShape * shape, const Genome * parent)
{
    (void)shape; (void)parent;
    abort();    /* stub */
}

int genome_validate(const Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}

void genome_free(Genome * genome)
{
    (void)genome;
    abort();    /* stub */
}
