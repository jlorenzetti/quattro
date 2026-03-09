/**
 * @file timing.h
 * @brief C64 frame pacing for Quattro.
 */

#ifndef QUATTRO_PLATFORM_C64_TIMING_H
#define QUATTRO_PLATFORM_C64_TIMING_H

/** Blocks until the next jiffy (next frame). Requires IRQ running. */
void timing_wait_frame(void);

#endif
