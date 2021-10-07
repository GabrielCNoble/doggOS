#include "k_rng.h"

uint32_t k_rng_seed = 0;

void k_rng_Seed(uint32_t seed)
{
    k_rng_seed = seed;
}

uint32_t k_rng_Rand()
{
    uint32_t value = k_rng_seed;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    k_rng_seed = value;
    return value; 
}