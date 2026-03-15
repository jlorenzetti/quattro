/**
 * @file input.h
 * @brief C64 keyboard polling for Quattro.
 *
 * Gameplay: A/D move, Z/X rotate, SPACE drop.
 * Setup/transitions: 0-9 set level, RETURN start / again.
 */

#ifndef QUATTRO_PLATFORM_C64_INPUT_H
#define QUATTRO_PLATFORM_C64_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include "types.h"

/** Polls keyboard once; returns Command or CMD_NONE. */
Command input_poll(void);

/** Returns true if any key was pressed (consumes the key). Used for title screen. */
bool input_any_key_poll(void);

/**
 * Polls for start/help screen: 0-9 set level, RETURN start, A decrement level, D increment level.
 * @param digit set to 0-9 if that key was pressed, else unchanged
 * @param start set to true if RETURN was pressed
 * @param level_down set to true if A was pressed (caller should decrement level, clamp 0)
 * @param level_up set to true if D was pressed (caller should increment level, clamp 9)
 */
void input_poll_start_help(uint8_t *digit, bool *start, bool *level_down, bool *level_up);

/** Returns true if RETURN was pressed (consumes the key). Used for replay flow. */
bool input_return_pressed(void);

#endif
