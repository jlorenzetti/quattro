/**
 * @file gravity.c
 * @brief Gravity interval table and lookup for Quattro.
 *
 * PAL-friendly gravity curve, adapted from a classic console pacing reference.
 * Table tuned for 50 Hz; levels 0–9 explicitly graded, 10+ follow the same
 * progression. Levels above the table use the last entry (fastest).
 */

#include "gravity.h"

static const uint8_t GRAVITY_FRAMES[] = {
    40, 36, 32, 28, 23, 19, 15, 11,  7,  5,  /* 0–9 */
     4,  4,  4,  3,  3,  3,  2,  2,  2,  2,  /* 10–19 */
     2,  2,  2,  2,  2,  2,  2,  2,  2,  1   /* 20–29; 30+ clamped */
};
#define GRAVITY_TABLE_LEN (sizeof(GRAVITY_FRAMES) / sizeof(GRAVITY_FRAMES[0]))

uint16_t gravity_interval_for_level(uint8_t level) {
    unsigned int idx = (level < GRAVITY_TABLE_LEN) ? (unsigned int)level : GRAVITY_TABLE_LEN - 1u;
    uint16_t n = (uint16_t)GRAVITY_FRAMES[idx];
    return (n >= 1) ? n : 1;
}
