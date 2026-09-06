/*
 * train/genome.c
 * Interface for mutation of genomes
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#include "../core/genome.h"


#include <stdlib.h>
#include <stdio.h>


// MARK: genome_init

Genome * genome_init(size_t address_space)
{
    if (address_space < 1)
    {
        fprintf(stderr, "genome_init: address_space must be at least 1\n");
        return NULL;
    }

    Nand * nands = calloc(1, sizeof(Nand));
    if (!nands)
    {
        perror("genome_init: Failed to calloc nands");
        return NULL;
    }

    Genome * genome = calloc(1, sizeof(Genome));
    if (!genome)
    {
        perror("genome_init: Failed to calloc genome");
        free(nands);
        return NULL;
    }

    genome->nands = nands;
    genome->num_nands = 0;
    genome->max_nands = 1;
    genome->address_space = address_space;

    return genome;
}


// MARK: genome_get_nand

Nand genome_get_nand(const Genome * genome, size_t nand_index)
{
    // Indices will be wrapped into the valid range
    if (genome->num_nands == 0) return (Nand){0};
    return genome->nands[nand_index % genome->num_nands];
}


// MARK: genome_set_nand

int genome_set_nand(Genome * genome, size_t nand_index, size_t input1_index, size_t input2_index, size_t output_index)
{
    if (genome->num_nands == 0) return -1;
    Nand new_nand = (Nand){
        .input1_index = input1_index % genome->address_space,
        .input2_index = input2_index % genome->address_space,
        .output_index = output_index % genome->address_space
    };
    genome->nands[nand_index % genome->num_nands] = new_nand;
    return 0;
}


// MARK: genome_add_nand

int genome_add_nand(Genome * genome, size_t input1_index, size_t input2_index, size_t output_index)
{
    if (genome->num_nands == genome->max_nands)
    {
        // Dynamic resize nands array
        size_t new_max_nands = genome->max_nands * 2; // 2^ growth

        Nand * new_nands = realloc(genome->nands, sizeof(Nand) * new_max_nands);
        if (!new_nands)
        {
            perror("genome_add_nand: Failed to realloc new_nands");
            return -1;
        }

        genome->nands = new_nands;
        genome->max_nands = new_max_nands;
    }

    Nand new_nand = (Nand){
        .input1_index = input1_index % genome->address_space,
        .input2_index = input2_index % genome->address_space,
        .output_index = output_index % genome->address_space
    };
    genome->nands[genome->num_nands++] = new_nand;
    return 0;
}


// MARK: genome_remove_nand

int genome_remove_nand(Genome * genome, size_t nand_index)
{
    if (genome->num_nands == 0) return -1;
    for (size_t i = nand_index % genome->num_nands; i + 1 < genome->num_nands; i++)
    {
        genome->nands[i] = genome->nands[i + 1];
    }
    genome->num_nands--;
    return 0;
}


// MARK: genome_free

void genome_free(Genome * genome)
{
    if (!genome) return;
    if (genome->nands) free(genome->nands);
    free(genome);
}
