/*
 * train/logger.c
 * Logger: the run log, one JSON object per line.
 *
 * Three functions because the log is a resource. Every component writes its
 * own events, and logging never changes results.
 *
 * Order is wall time, and never out of generation order. Per-thread buffers
 * are merged at generation boundaries, which is the rest point that makes the
 * guarantee hold: a generation's lines can never appear among the next one's,
 * while within a generation the order follows whichever thread got there
 * first, because an example's index is arbitrary and may be produced in
 * parallel. The log is therefore not one of the things compared between runs.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdarg.h>
#include <stdlib.h>

int logger_open(const char * out_dir)
{
    (void)out_dir;
    abort();    /* stub */
}

void logger_event(uint32_t generation, const char * component,
                  const char * event, const char * fmt, ...)
{
    (void)generation; (void)component; (void)event; (void)fmt;
    abort();    /* stub */
}

void logger_close(void)
{
    abort();    /* stub */
}
