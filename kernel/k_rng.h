#ifndef K_RNG_H
#define K_RNG_H

#include <stdint.h>

void k_rng_Seed(uint32_t seed);

uint32_t k_rng_Rand();

#endif