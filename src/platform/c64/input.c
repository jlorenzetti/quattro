/**
 * @file input.c
 * @brief C64 keyboard polling for Quattro.
 */

#include <stddef.h>

#include "input.h"
#include "input_model.h"

/** KERNAL: scan keyboard, fill buffer. */
#define SCNKEY ((void (*)(void))0xFF9F)
/** KERNAL: get one character from buffer; 0 if empty. */
#define GETIN  ((unsigned char (*)(void))0xFFE4)

/** CIA1: keyboard matrix. Write row (one bit low) to $DC00, read columns from $DC01; 0 = pressed. */
#define CIA1_PRA   (*(volatile unsigned char *)0xDC00)
#define CIA1_PRB   (*(volatile unsigned char *)0xDC01)

/** C64 matrix: A = row 1, col 2; D = row 2, col 2 (C64-Wiki). */
static bool key_held_a(void) {
    CIA1_PRA = (unsigned char)~0x02u;  /* row 1 */
    return (CIA1_PRB & 0x04u) == 0u;
}
static bool key_held_d(void) {
    CIA1_PRA = (unsigned char)~0x04u;  /* row 2 */
    return (CIA1_PRB & 0x04u) == 0u;
}

static InputModel g_input_model;

void input_init(void) {
    input_model_init(&g_input_model);
}

void input_reset_gameplay(void) {
    input_model_reset(&g_input_model);
}

Command input_poll(void) {
    SCNKEY();
    {
        /* For now, we still use GETIN to detect Z/X/SPACE presses. */
        unsigned char c = GETIN();
        InputFrame frame = {0};

        /* Lateral held state uses CIA matrix (independent of KERNAL repeat). */
        CIA1_PRA = 0xFFu;
        frame.move_left_held = key_held_a();
        CIA1_PRA = 0xFFu;
        frame.move_right_held = key_held_d();
        CIA1_PRA = 0xFFu;

        if (c != 0) {
            if (c == 'A') frame.move_left_held = true;
            else if (c == 'D') frame.move_right_held = true;
            else if (c == 'Z') frame.rotate_ccw_held = true;
            else if (c == 'X') frame.rotate_cw_held = true;
            else if (c == ' ') frame.soft_drop_held = true;
        }

        return input_model_step(&g_input_model, &frame);
    }
}

bool input_any_key_poll(void) {
    SCNKEY();
    return GETIN() != 0;
}

/** PETSCII RETURN. */
#define PETSCII_RETURN 13u
/** PETSCII '0'..'9'. */
#define PETSCII_0      0x30u
#define PETSCII_9      0x39u

void input_poll_start_help(uint8_t *digit, bool *start, bool *level_down, bool *level_up) {
    SCNKEY();
    {
        unsigned char c = GETIN();
        if (c == 0) return;
        if (c == PETSCII_RETURN) {
            *start = true;
            return;
        }
        if (c == 'A' && level_down != NULL) {
            *level_down = true;
            return;
        }
        if (c == 'D' && level_up != NULL) {
            *level_up = true;
            return;
        }
        if (c >= PETSCII_0 && c <= PETSCII_9 && digit != NULL) {
            *digit = (uint8_t)(c - PETSCII_0);
        }
    }
}

bool input_return_pressed(void) {
    SCNKEY();
    return GETIN() == PETSCII_RETURN;
}
