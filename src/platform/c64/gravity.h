/**
 * @file gravity.h
 * @brief Level-to-interval gravity curve for Quattro (C64).
 *
 * PAL-friendly gravity curve, adapted from a classic console pacing reference.
 * Provides frames-between-drops per level for PAL 50 Hz (C64 jiffy).
 */

#ifndef QUATTRO_PLATFORM_C64_GRAVITY_H
#define QUATTRO_PLATFORM_C64_GRAVITY_H

#include <stdint.h>

/**
 * Returns the number of frames to wait between automatic gravity ticks for
 * the given level. Levels above the table range are clamped to the last
 * (fastest) entry. Minimum return value is 1.
 *
 * Runtime assumption: PAL 50 Hz (C64 jiffy). The table is tuned so that
 * level 0 feels slow and level 9 feels deliberately fast, with a severe
 * but readable progression.
 */
uint16_t gravity_interval_for_level(uint8_t level);

#endif
