/**
 * @file seed.c
 * @brief C64 seed generation: jiffy-based pseudorandom or compile-time fixed seed.
 */

#include "seed.h"

#ifdef QUATTRO_FIXED_SEED

uint32_t seed_generate(uint8_t start_level, uint16_t frames_at_start) {
    (void)start_level;
    (void)frames_at_start;
    return (uint32_t)QUATTRO_FIXED_SEED;
}

#else

/** C64 jiffy clock (zero page); same address family as timing.c (0xA2 = LSB). */
#define JIFFY_L  (*(volatile uint8_t *)0xA2)
#define JIFFY_M  (*(volatile uint8_t *)0xA1)
#define JIFFY_H  (*(volatile uint8_t *)0xA0)

uint32_t seed_generate(uint8_t start_level, uint16_t frames_at_start) {
    uint32_t seed = 0;

    seed |= (uint32_t)JIFFY_L;
    seed |= (uint32_t)JIFFY_M << 8;
    seed |= (uint32_t)JIFFY_H << 16;
    seed ^= (uint32_t)frames_at_start << 8;
    seed ^= (uint32_t)(frames_at_start >> 8);

    seed ^= (uint32_t)start_level << 24;
    seed ^= seed << 7;
    seed ^= seed >> 9;
    seed ^= 0xA5A5u;

    if (seed == 0) {
        seed = 1;
    }

    return seed;
}

#endif
