/*
 * tests/test_canonical.c
 * A genome and its canonical form behave identically.
 *
 * Canonicalisation drops each Nand's output index, prunes Nands whose output
 * wire an older Nand already drives, and renumbers. None of that may change
 * what the genome computes, on any example, for any number of ticks.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */

#include <stdlib.h>

int main(void)
{
    abort();    /* stub */
}
