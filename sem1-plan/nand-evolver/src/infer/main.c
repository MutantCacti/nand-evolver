/*
 * infer/main.c
 * Deployment: the model compiled in, and nothing else. For each input record on stdin it writes one output record on stdout, lock-step. The model always answers: at the tick limit it emits whatever its output region holds, exactly as training scored it.

 * One process runs one example by default, so starting a new process clears
 * the memory space. This file owns stdin and stdout, and is the whole of what
 * the deployed program adds to the shared components: it builds a Feed over
 * them and hands it to the Harness, which knows nothing of either.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/arena.h"
#include "../core/harness.h"
#include "../core/model.h"

#include <stdlib.h>

int main(void)
{
    abort();    /* stub */
}
