/**
 * @file seed.h
 * @brief C64 runtime seed generation for mainline gameplay (pseudorandom) or fixed seed for debug builds.
 */

#ifndef QUATTRO_C64_SEED_H
#define QUATTRO_C64_SEED_H

#include <stdint.h>

/**
 * Returns a seed for the game RNG.
 * Mainline: jiffy clock, start_level, and frames spent on start/help before RETURN.
 * Debug: fixed value when QUATTRO_FIXED_SEED is defined at compile time.
 *
 * @param start_level initial level (0–9)
 * @param frames_at_start frames spent on start/help before RETURN
 * @return non-zero seed for game_start()
 */
uint32_t seed_generate(uint8_t start_level, uint16_t frames_at_start);

#endif
