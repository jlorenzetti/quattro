/**
 * @file timing.c
 * @brief C64 frame pacing for Quattro.
 */

#include "timing.h"

/** C64 jiffy clock LSB (zero page); updated by system IRQ at ~60Hz after KERNAL `IOINIT`. */
#define JIFFY_LSB (*(volatile unsigned char *)0xA2)

/**
 * @brief Wait one frame (~jiffy at `$A2`). Issues `cli` so IRQ-driven jiffy advances (KERNAL may have left `sei`).
 */
void timing_wait_frame(void) {
    __asm__ volatile("cli" ::: "memory");
    unsigned char t = JIFFY_LSB;
    while (JIFFY_LSB == t) {
        /* wait for change */
    }
}
