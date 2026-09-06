/*
 * train/rng.h
 * Pseudorandom generator.
 *
 * Training only: run/ performs deterministic inference and draws nothing.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#ifndef RNG_H_
#define RNG_H_


#include <stdint.h>


typedef struct
{
    uint64_t state;
}
Rng;


// Seed a generator directly
void rng_seed(Rng * rng, uint64_t seed);


// Derive an independent random stream from a base seed and a position index
Rng rng_stream(uint64_t seed, uint64_t index);


// Next raw 64 bits
uint64_t rng_next(Rng * rng);


// Uniform in [0, bound), returns 0 when bound < 2
uint64_t rng_below(Rng * rng, uint64_t bound);


// Uniform in [0, 1), using 53 bits of mantissa
double rng_unit(Rng * rng);


// True with probability p; p <= 0 never, p >= 1 always
int rng_chance(Rng * rng, double p);


// Uniform in [min, max], inclusive at both ends. Returns min if max < min
int64_t rng_range(Rng * rng, int64_t min, int64_t max);


#endif
