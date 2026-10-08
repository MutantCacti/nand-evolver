/*
 * train/logger.c
 * Logger: the run log. Every component writes its own events. Per-thread buffers, merged in canonical order at generation boundaries, so logging never changes a result.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

Logger * logger_open(const char * path, size_t threads)
{
    (void)path; (void)threads;
    abort();    /* stub */
}

void logger_event(Logger * logger, size_t thread, const char * event, const char * fields)
{
    (void)logger; (void)thread; (void)event; (void)fields;
    abort();    /* stub */
}

int logger_merge(Logger * logger, size_t generation)
{
    (void)logger; (void)generation;
    abort();    /* stub */
}

int logger_close(Logger * logger)
{
    (void)logger;
    abort();    /* stub */
}
