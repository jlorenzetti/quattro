/**
 * @file input.c
 * @brief C64 keyboard polling for Quattro.
 */

#include "input.h"

/** KERNAL: scan keyboard, fill buffer. */
#define SCNKEY ((void (*)(void))0xFF9F)
/** KERNAL: get one character from buffer; 0 if empty. */
#define GETIN  ((unsigned char (*)(void))0xFFE4)

Command input_poll(void) {
    SCNKEY();
    {
        unsigned char c = GETIN();
        if (c == 0) return CMD_NONE;
        if (c == 'A') return CMD_MOVE_LEFT;
        if (c == 'D') return CMD_MOVE_RIGHT;
        if (c == 'Z') return CMD_ROTATE_CCW;
        if (c == 'X') return CMD_ROTATE_CW;
        if (c == ' ') return CMD_SOFT_DROP;
    }
    return CMD_NONE;
}
