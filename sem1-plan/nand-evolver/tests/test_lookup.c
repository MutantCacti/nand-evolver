/*
 * tests/test_lookup.c
 * Example lookup is pure.
 *
 * Writes a small Dataset file in the layout train.h specifies, opens it, and
 * checks that:
 *
 *   - the header is read back as written
 *   - the same (seed, generation, position) always names the same example,
 *     whatever was looked up before it, so no order is stored or walked
 *   - an example's inputs and its expected bits belong to the same example
 *   - every lookup lands on an example of the file, never between two
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../src/train/train.h"
#include "check.h"

#include <stdio.h>
#include <string.h>

#define NUM_EXAMPLES 8u
#define POSITIONS    32u

/* Example e: round 0 carries e, round 1 carries 7 - e, round 1 is graded and
 * expects e & 3. So any example can be identified from its first byte, and
 * its other two bytes checked against it. */
static int write_dataset(const char * path)
{
    FILE * file = fopen(path, "wb");
    if (!file)
        return 1;
    const uint32_t header[] = { 3, 2, 2, NUM_EXAMPLES, 1 };    /* i, m, rounds, examples, graded */
    fwrite("NDS1", 1, 4, file);
    for (size_t k = 0; k < 5; k++) {
        uint8_t le[4] = { (uint8_t)header[k], (uint8_t)(header[k] >> 8),
                          (uint8_t)(header[k] >> 16), (uint8_t)(header[k] >> 24) };
        fwrite(le, 1, 4, file);
    }
    fputc(0x02, file);                                          /* round 1 graded */
    for (uint8_t e = 0; e < NUM_EXAMPLES; e++) {
        fputc(e, file);
        fputc(7 - e, file);
        fputc(e & 3, file);
    }
    return fclose(file);
}

static uint8_t identify(const Dataset * dataset, uint64_t seed, uint32_t generation, uint32_t position)
{
    const uint8_t * expected = NULL;
    const uint8_t * inputs = dataset_example(dataset, seed, generation, position, &expected);
    CHECK(inputs != NULL && expected != NULL);
    CHECK(inputs[0] < NUM_EXAMPLES);
    CHECK(inputs[1] == 7 - inputs[0]);
    CHECK(expected[0] == (inputs[0] & 3));
    return inputs[0];
}

int main(void)
{
    const char * path = "test_lookup.ds";
    CHECK(write_dataset(path) == 0);

    Dataset dataset;
    memset(&dataset, 0, sizeof dataset);
    CHECK(dataset_open(&dataset, path) == 0);
    CHECK(dataset.num_inputs == 3);
    CHECK(dataset.num_outputs == 2);
    CHECK(dataset.rounds == 2);
    CHECK(dataset.num_examples == NUM_EXAMPLES);
    CHECK(dataset.num_graded == 1);
    CHECK(dataset.graded[0] == 0x02);

    /* Forward, then backward: a lookup that walked a stored order would differ. */
    uint8_t forward[POSITIONS];
    for (uint32_t p = 0; p < POSITIONS; p++)
        forward[p] = identify(&dataset, 7, 3, p);
    for (uint32_t p = POSITIONS; p-- > 0;)
        CHECK(identify(&dataset, 7, 3, p) == forward[p]);

    /* Interleaved with other seeds and generations, still the same. */
    for (uint32_t p = 0; p < POSITIONS; p++) {
        identify(&dataset, 8, 3, p);
        identify(&dataset, 7, 4, p);
        CHECK(identify(&dataset, 7, 3, p) == forward[p]);
    }

    dataset_close(&dataset);
    remove(path);
    return check_failures != 0;
}
