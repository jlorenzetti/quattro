/**
 * @file timing.c
 * @brief C64 frame pacing for Quattro.
 */

#include "timing.h"

/** C64 jiffy clock LSB (zero page); updated by system IRQ at ~60Hz. */
#define JIFFY_LSB  (*(volatile unsigned char *)0xA2)

void timing_wait_frame(void) {
    unsigned char t = JIFFY_LSB;
    while (JIFFY_LSB == t) { /* wait for change */ }
}
