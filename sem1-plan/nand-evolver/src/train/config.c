/*
 * train/config.c
 * Reading the experiment file into the read-only global.
 *
 * The flat `key = value` format, dotted keys. Only the `parameter.` keys and
 * `execution.threads` are read here; every other key was compiled in, and
 * `task.` keys are never read by C at all — everything train needs about the
 * task comes from the Dataset header, so compiling it in as well would make
 * two sources of truth that can disagree.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

static Config loaded;
const Config * config = NULL;

int config_load(const char * path)
{
    (void)path; (void)loaded;
    abort();    /* stub */
}
