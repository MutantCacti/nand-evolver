/*
 * tests/check.h
 * The C tests' one assertion. A failed CHECK reports itself and the test
 * continues, so one run shows every failure; main returns the count.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef TESTS_CHECK_H
#define TESTS_CHECK_H

#include <stdio.h>

static int check_failures = 0;

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #condition); \
            check_failures++; \
        } \
    } while (0)

#endif
