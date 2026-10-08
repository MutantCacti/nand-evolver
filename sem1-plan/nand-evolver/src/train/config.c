/*
 * train/config.c
 * Reads the flat key = value experiment file, execution keys included, and
 * refuses one whose build hash differs from the hash compiled into this binary.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int config_load(Config * config, const char * path)
{
    (void)config; (void)path;
    abort();    /* stub */
}
