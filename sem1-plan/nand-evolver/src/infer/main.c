/*
 * infer/main.c
 * Deployment: the model compiled in, and nothing else. For each input record on stdin it writes one output record on stdout, lock-step. The model always answers: at the tick limit it emits whatever its output region holds, exactly as training scored it.

 * One process runs one example by default, so starting a new process clears
 * the memory space. This file owns stdin and stdout; core knows nothing of
 * them, and supplies them to the Harness as a DeploymentIO.
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
