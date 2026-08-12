#ifndef ASTEROIDS_CORE_RNG_H
#define ASTEROIDS_CORE_RNG_H

#include <stdint.h>

typedef struct Rng {
    uint32_t state;
} Rng;

static inline void rng_seed(Rng *rng, uint32_t seed) {
    rng->state = seed != 0 ? seed : 1u;
}

static inline uint32_t rng_next_u32(Rng *rng) {
    uint32_t x = rng->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng->state = x;
    return x;
}

static inline float rng_next_float01(Rng *rng) {
    return (float)(rng_next_u32(rng) >> 8) / (float)(1u << 24);
}

static inline float rng_range(Rng *rng, float min_value, float max_value) {
    return min_value + rng_next_float01(rng) * (max_value - min_value);
}

#endif
