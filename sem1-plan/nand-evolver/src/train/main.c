/*
 * train/main.c
 * One Run per invocation: the experiment file, a seed, a Dataset, an out dir.
 *
 *     train --seed <u64> --dataset <path> --out <dir> [--config <path>]
 *
 * The Driver repeats this per seed, because the loop over Runs is the Driver's
 * and nothing in C should hold it. --config overrides the copy embedded in
 * this binary and is refused unless its build hash matches; without it the
 * embedded copy is used, which is the path that carries reproducibility.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "train.h"

#include <stdlib.h>

int main(int argc, char ** argv)
{
    (void)argc; (void)argv;
    abort();    /* stub */
}
