/*
 * tests/test_resume.c
 * Resuming is indistinguishable from never stopping.
 *
 * Run to a generation boundary, checkpoint, stop, read the checkpoint back and
 * continue. The result must be bit-identical to an uninterrupted run: the same
 * model file, the same checkpoints and the same per-generation records.
 *
 * This holds because randomness is pure in (seed, level indices), so a
 * checkpoint needs only the generation index and the population.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */

#include <stdlib.h>

int main(void)
{
    abort();    /* stub */
}
