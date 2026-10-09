/*
 * infer/main.c
 * Deployment: one model, running for as long as it is switched on.
 *
 * It owns the only loop in the product, over records on stdin: on an input
 * record it calls the Harness for one round and writes one output record, and
 * on a reset record it has the Harness reset the memory space. The Harness
 * returns the output region, so packing those words into a record's bits is
 * all this file does with the memory space — it never computes an offset into
 * it, because where the output region begins is the Harness's business. Because it
 * returns between rounds, an embedder may choose a round's input after seeing
 * the previous round's output.
 *
 * An example is the span between resets, not a level: nothing here loops over
 * examples. Starting a process is itself a reset, so running one process per
 * example sends no reset records at all, and a long-lived process serving many
 * examples sends them. `main` may own this loop — the rule against data owning
 * loops exists to stop a struct being in charge, and the program acts.
 *
 * The memory space is created once, here, because there is no level above it
 * to own one. There is no Config, no Selector, Mutator, Trainer or Verifier,
 * and no command line: the model is compiled in. The generated header is
 * model_data.h rather than model.h so that its basename cannot collide with
 * core/model.h, which is a different file that this program never includes.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../core/harness.h"
#include "records.h"

#include "model_data.h" /* generated per build: the canonical genome as Nands */

#include <stdlib.h>

/* The whole memory space in static storage, sized from the compiled-in model,
 * so a deployment makes no allocation at all. Not configured: P1 has no reason
 * to allocate one dynamically. */
static word arena_wires[MODEL_NUM_WIRES];

int main(void)
{
    (void)arena_wires;
    abort();    /* stub */
}
