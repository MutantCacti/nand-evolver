/*
 * train/rng.c
 * splitmix64 pseudorandom generator.
 * https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#include "rng.h"


#define GOLDEN_GAMMA 0x9E3779B97F4A7C15ULL


static uint64_t mix64(uint64_t z)
{
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}


// MARK: rng_seed

void rng_seed(Rng * rng, uint64_t seed)
{
    if (!rng) return;
    rng->state = seed;
}


// MARK: rng_stream

Rng rng_stream(uint64_t seed, uint64_t index)
{
    Rng rng;
    rng.state = mix64(seed ^ mix64(index + GOLDEN_GAMMA));
    return rng;
}


// MARK: rng_next

uint64_t rng_next(Rng * rng)
{
    if (!rng) return 0;
    return mix64(rng->state += GOLDEN_GAMMA);
}


// MARK: rng_below

uint64_t rng_below(Rng * rng, uint64_t bound)
{
    if (!rng || bound < 2) return 0;

    /* Modulo is biased towards smaller values when the operand
     * does not divide 2^64. Reject samples below 2^64 % bound;
     * the sampling interval is then a multiple of bound,
     * eliminating the modulo bias.
     *
     * This particular form is required to divide 2^64 with a
     * 64-bit type:
     *
     *      0 - bound (overlow) = 2^64 - bound
     *      (2^64 - bound) === 2^64 % bound
     *      .'. (2^64 - bound) % bound = 2^64 % bound
     */
    uint64_t threshold = (0 - bound) % bound;

    uint64_t r;
    do {
        r = rng_next(rng);
    } while (r < threshold);

    return r % bound;
}


// MARK: rng_unit

double rng_unit(Rng * rng)
{
    if (!rng) return 0.0;

    // Top 53 bits: exactly the mantissa width of a double
    return (double)(rng_next(rng) >> 11) * 0x1.0p-53;
}


// MARK: rng_chance

int rng_chance(Rng * rng, double p)
{
    if (!rng || !(p > 0.0)) return 0;   // also rejects NaN
    if (p >= 1.0) return 1;

    return rng_unit(rng) < p;
}


// MARK: rng_range

int64_t rng_range(Rng * rng, int64_t min, int64_t max)
{
    if (!rng || max < min) return min;

    uint64_t span = (uint64_t)max - (uint64_t)min;
    if (span == UINT64_MAX) return (int64_t)rng_next(rng);

    return min + (int64_t)rng_below(rng, span + 1);
}
