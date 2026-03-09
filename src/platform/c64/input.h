/**
 * @file input.h
 * @brief C64 keyboard polling for Quattro.
 *
 * Keymap (PETSCII): A=left, D=right, Z=rotate CCW, X=rotate CW, SPACE=soft drop.
 */

#ifndef QUATTRO_PLATFORM_C64_INPUT_H
#define QUATTRO_PLATFORM_C64_INPUT_H

#include "types.h"

/** Polls keyboard once; returns Command or CMD_NONE. */
Command input_poll(void);

#endif
