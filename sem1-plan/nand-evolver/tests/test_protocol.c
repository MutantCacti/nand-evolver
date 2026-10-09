/*
 * tests/test_protocol.c
 * The README's semantics, through the Harness and the Kernel only.
 *
 * Every genome here is built by hand, and wires are named by the README's
 * layout, which this test is entitled to know because it is what is under
 * test. Each case is the smallest genome that shows one rule:
 *
 *   - the first tick runs before ready is checked, and ready ends the round
 *   - a round that never signals ends at the tick limit with its outputs as
 *     they stand (forced closure)
 *   - a tick reads the memory space as it stood when the tick began
 *   - writeback is in reverse Nand index order: the lowest index wins
 *   - ready's start value is written at the start of every round
 *   - memory persists across the rounds of an example and is reset between
 *     examples, to an initial state when one is given
 *   - an example run whole resets first, then carries memory across its rounds
 *   - inputs are written every round
 *   - genome_validate refuses writes to the constant wire or the inputs
 *   - after every round, every wire is all zeroes or all ones, and the
 *     constant wire is zero
 *
 * Written for the reference protocol: ready starts at 0 and 1 means ready.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../src/core/harness.h"
#include "check.h"

#include <stdlib.h>

#if !defined(PROTOCOL_KERNEL_REFERENCE) || PROTOCOL_READY_START != 0 || PROTOCOL_READY_VALUE != 1
#error "test_protocol is written for the reference kernel with ready_start = 0 and ready_value = 1"
#endif

/* The README's layout, for a genome with i inputs and m outputs. */
#define CONSTANT        0u
#define INPUT(k)        (1u + (k))
#define READY(i)        (1u + (i))
#define OUTPUT(i, k)    (2u + (i) + (k))
#define INTERNAL(i, m, k) (2u + (i) + (m) + (k))

typedef struct
{
    Genome * genome;
    Arena arena;
}
Fixture;

static Fixture fixture(uint32_t i, uint32_t m, uint32_t internal, const Nand * nands, uint32_t n)
{
    GenomeShape shape = { i, m, internal, 16 };
    Fixture f;
    f.genome = genome_create(&shape);
    for (uint32_t k = 0; k < n; k++)
        f.genome->nands[k] = nands[k];
    f.genome->num_nands = n;
    f.arena.num_wires = GENOME_NUM_WIRES(f.genome);
    f.arena.wires = calloc(f.arena.num_wires, sizeof(word));
    harness_reset(f.genome, &f.arena, NULL);
    return f;
}

static void release(Fixture * f)
{
    free(f->arena.wires);
    genome_free(f->genome);
}

/* Every wire all zeroes or all ones, and the constant wire zero. */
static void check_words(const Arena * arena)
{
    CHECK(arena->wires[CONSTANT] == 0);
    for (size_t w = 0; w < arena->num_wires; w++)
        CHECK(arena->wires[w] == 0 || arena->wires[w] == WORD_ALL);
}

static const word * round_of(Fixture * f, uint8_t inputs, uint32_t tick_limit, uint32_t * ticks)
{
    const word * out = harness_round(f->genome, &f->arena, &inputs, tick_limit, ticks);
    check_words(&f->arena);
    return out;
}

static void ready_ends_the_round_after_the_first_tick(void)
{
    Nand nands[] = { { CONSTANT, CONSTANT, READY(1) } };     /* ~(0 & 0) = 1 */
    Fixture f = fixture(1, 1, 0, nands, 1);
    uint32_t ticks = 0;
    round_of(&f, 0, 8, &ticks);
    CHECK(ticks == 1);
    release(&f);
}

static void a_silent_round_ends_at_the_tick_limit(void)
{
    Fixture f = fixture(1, 1, 0, NULL, 0);
    uint32_t ticks = 0;
    const word * out = round_of(&f, 1, 5, &ticks);
    CHECK(ticks == 5);
    CHECK(out[0] == 0);
    release(&f);
}

static void a_tick_reads_the_memory_as_it_began(void)
{
    /* A := ~(0 & 0) = 1, and OUT := ~(A & A), which sees A's old value on tick 1. */
    Nand nands[] = {
        { CONSTANT, CONSTANT, INTERNAL(1, 1, 0) },
        { INTERNAL(1, 1, 0), INTERNAL(1, 1, 0), OUTPUT(1, 0) },
    };
    Fixture f = fixture(1, 1, 1, nands, 2);
    uint32_t ticks = 0;
    CHECK(round_of(&f, 0, 1, &ticks)[0] == WORD_ALL);
    harness_reset(f.genome, &f.arena, NULL);
    CHECK(round_of(&f, 0, 2, &ticks)[0] == 0);
    release(&f);
}

static void the_lowest_index_wins_a_collision(void)
{
    /* With input 1: ~(0 & 0) = 1 and ~(in & in) = 0 both write OUT. */
    Nand senior_one[] = {
        { CONSTANT, CONSTANT, OUTPUT(1, 0) },
        { INPUT(0), INPUT(0), OUTPUT(1, 0) },
        { CONSTANT, CONSTANT, READY(1) },
    };
    Nand senior_zero[] = {
        { INPUT(0), INPUT(0), OUTPUT(1, 0) },
        { CONSTANT, CONSTANT, OUTPUT(1, 0) },
        { CONSTANT, CONSTANT, READY(1) },
    };
    uint32_t ticks = 0;

    Fixture f = fixture(1, 1, 0, senior_one, 3);
    CHECK(round_of(&f, 1, 8, &ticks)[0] == WORD_ALL);
    release(&f);

    f = fixture(1, 1, 0, senior_zero, 3);
    CHECK(round_of(&f, 1, 8, &ticks)[0] == 0);
    release(&f);
}

static void ready_starts_every_round_at_its_start_value(void)
{
    /* READY := ~(READY & READY) toggles. Started at 0 it answers on tick 1; if
     * the second round inherited 1 instead, it would take two ticks. */
    Nand nands[] = { { READY(1), READY(1), READY(1) } };
    Fixture f = fixture(1, 1, 0, nands, 1);
    uint32_t ticks = 0;
    round_of(&f, 0, 8, &ticks);
    CHECK(ticks == 1);
    round_of(&f, 0, 8, &ticks);
    CHECK(ticks == 1);
    release(&f);
}

static void memory_persists_across_rounds_and_resets_between_examples(void)
{
    /* OUT := ~(OUT & OUT) toggles once per round of one tick. */
    Nand nands[] = { { OUTPUT(1, 0), OUTPUT(1, 0), OUTPUT(1, 0) } };
    Fixture f = fixture(1, 1, 0, nands, 1);
    uint32_t ticks = 0;
    CHECK(round_of(&f, 0, 1, &ticks)[0] == WORD_ALL);
    CHECK(round_of(&f, 0, 1, &ticks)[0] == 0);
    harness_reset(f.genome, &f.arena, NULL);
    CHECK(round_of(&f, 0, 1, &ticks)[0] == WORD_ALL);
    release(&f);
}

static void an_example_is_a_lifetime(void)
{
    /* OUT := ~(OUT & OUT) toggles once per one-tick round: 1, 0, 1. The second
     * call answers the same, so the example began with a reset. */
    Nand nands[] = { { OUTPUT(1, 0), OUTPUT(1, 0), OUTPUT(1, 0) } };
    Fixture f = fixture(1, 1, 0, nands, 1);
    uint8_t inputs[3] = { 0, 0, 0 };
    for (int call = 0; call < 2; call++) {
        word outputs[3];
        uint32_t ticks[3 * EXECUTION_LANE_WIDTH];
        harness_example(f.genome, &f.arena, NULL, inputs, 3, 1, outputs, ticks);
        check_words(&f.arena);
        CHECK(outputs[0] == WORD_ALL);
        CHECK(outputs[1] == 0);
        CHECK(outputs[2] == WORD_ALL);
        CHECK(ticks[0] == 1 && ticks[1] == 1 && ticks[2] == 1);
    }
    release(&f);
}

static void inputs_are_written_every_round(void)
{
    Nand nands[] = { { INPUT(0), INPUT(0), OUTPUT(1, 0) } };
    Fixture f = fixture(1, 1, 0, nands, 1);
    uint32_t ticks = 0;
    CHECK(round_of(&f, 1, 1, &ticks)[0] == 0);
    CHECK(round_of(&f, 0, 1, &ticks)[0] == WORD_ALL);
    release(&f);
}

static void a_reset_sets_the_initial_state_when_given(void)
{
    /* OUT := ~(INTERNAL3 & INTERNAL3). Internal bit 3 set → OUT = 0. */
    Nand nands[] = { { INTERNAL(1, 1, 3), INTERNAL(1, 1, 3), OUTPUT(1, 0) } };
    Fixture f = fixture(1, 1, 8, nands, 1);
    uint8_t initial = 0x08;
    uint32_t ticks = 0;
    harness_reset(f.genome, &f.arena, &initial);
    check_words(&f.arena);
    CHECK(round_of(&f, 0, 1, &ticks)[0] == 0);
    harness_reset(f.genome, &f.arena, NULL);
    CHECK(round_of(&f, 0, 1, &ticks)[0] == WORD_ALL);
    release(&f);
}

static void validation_refuses_writes_to_reserved_wires(void)
{
    Nand to_constant[] = { { CONSTANT, CONSTANT, CONSTANT } };
    Nand to_input[] = { { CONSTANT, CONSTANT, INPUT(0) } };
    Nand out_of_range[] = { { 99, CONSTANT, OUTPUT(1, 0) } };
    Nand to_ready[] = { { INPUT(0), CONSTANT, READY(1) } };

    Fixture f = fixture(1, 1, 0, to_constant, 1);
    CHECK(genome_validate(f.genome) != 0);
    release(&f);
    f = fixture(1, 1, 0, to_input, 1);
    CHECK(genome_validate(f.genome) != 0);
    release(&f);
    f = fixture(1, 1, 0, out_of_range, 1);
    CHECK(genome_validate(f.genome) != 0);
    release(&f);
    f = fixture(1, 1, 0, to_ready, 1);
    CHECK(genome_validate(f.genome) == 0);
    release(&f);
}

int main(void)
{
    ready_ends_the_round_after_the_first_tick();
    a_silent_round_ends_at_the_tick_limit();
    a_tick_reads_the_memory_as_it_began();
    the_lowest_index_wins_a_collision();
    ready_starts_every_round_at_its_start_value();
    memory_persists_across_rounds_and_resets_between_examples();
    an_example_is_a_lifetime();
    inputs_are_written_every_round();
    a_reset_sets_the_initial_state_when_given();
    validation_refuses_writes_to_reserved_wires();
    return check_failures != 0;
}
