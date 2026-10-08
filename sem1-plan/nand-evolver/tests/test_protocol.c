/*
 * tests/test_protocol.c
 * The README's semantics, independent of layout:
 *
 *   - wire 0 reads 0 always, and nothing may drive it
 *   - the input region is reserved: no Nand may name an input wire as output
 *   - ready takes its start value at the start of every round, with the inputs
 *   - ready is checked after each tick and never before the first, so every
 *     round runs at least one tick and the start value alone cannot answer
 *   - a round ends at the tick limit with whatever the output region holds
 *   - writeback is in reverse Nand index order, so the lowest-indexed Nand
 *     wins a collision and a junior Nand on a driven wire never takes effect
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */

#include <stdlib.h>

int main(void)
{
    abort();    /* stub */
}
