/*
 * tests/probe.c
 * A fixture, not a test: runs a hand-written genome through train's path and
 * exports it, so test_records.py can run the exported model through infer and
 * compare the two.
 *
 *     probe <out_dir> < script
 *
 * The script is text, one command per line:
 *
 *     shape <inputs> <outputs> <internal>     first line
 *     nand <a> <b> <out>                      the genome's Nands, in index order
 *     reset                                   harness_reset with no initial state
 *     round <hex>                             harness_round with packed input bytes
 *
 * Each round prints its output region as packed hex bytes on one line. At the
 * end of the script the genome is handed to exporter_export, which
 * canonicalises it and writes <out_dir>/model.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#include "../src/train/train.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t hex_bytes(const char * hex, uint8_t * bytes, size_t max)
{
    size_t n = 0;
    unsigned value;
    while (n < max && sscanf(hex + 2 * n, "%2x", &value) == 1)
        bytes[n++] = (uint8_t)value;
    return n;
}

int main(int argc, char ** argv)
{
    if (argc != 2)
        return 2;

    char line[256];
    GenomeShape shape = { 0, 0, 0, 0 };
    if (!fgets(line, sizeof line, stdin)
        || sscanf(line, "shape %u %u %u", &shape.num_inputs, &shape.num_outputs, &shape.num_internal) != 3)
        return 2;
    shape.max_nands = 1 + shape.num_outputs + shape.num_internal + 16;

    Genome * genome = genome_create(&shape, NULL);
    Arena arena = { NULL, GENOME_NUM_WIRES(genome) };
    arena.wires = calloc(arena.num_wires, sizeof(word));
    harness_reset(genome, &arena, NULL);

    while (fgets(line, sizeof line, stdin)) {
        Nand nand;
        char hex[128];
        if (sscanf(line, "nand %u %u %u", &nand.a, &nand.b, &nand.out) == 3) {
            genome->nands[genome->num_nands++] = nand;
        } else if (strncmp(line, "reset", 5) == 0) {
            harness_reset(genome, &arena, NULL);
        } else if (sscanf(line, "round %127s", hex) == 1) {
            uint8_t inputs[64] = { 0 };
            uint32_t ticks;
            hex_bytes(hex, inputs, sizeof inputs);
            const word * out = harness_round(genome, &arena, inputs, 1u << 16, &ticks);
            for (uint32_t byte = 0; byte < (genome->num_outputs + 7) / 8; byte++) {
                uint8_t packed = 0;
                for (uint32_t bit = 0; bit < 8 && 8 * byte + bit < genome->num_outputs; bit++)
                    packed |= (uint8_t)((out[8 * byte + bit] & 1u) << bit);
                printf("%02x", packed);
            }
            printf("\n");
        }
    }

    int failed = genome_validate(genome) != 0 || exporter_export(genome, argv[1]) != 0;
    free(arena.wires);
    genome_free(genome);
    return failed;
}
