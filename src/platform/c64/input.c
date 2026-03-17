/**
 * @file input.c
 * @brief C64 keyboard and joystick 2 input for Quattro.
 *
 * Joystick is supported across the full loop: title (any dir/fire), start/help
 * (left/right = level, fire = start), gameplay (left/right/down/fire = move/drop/rotate),
 * game over (fire = return to start/help).
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

/** CIA1 data direction registers (0 = input, 1 = output). */
#define CIA1_DDRA  (*(volatile unsigned char *)0xDC02)
#define CIA1_DDRB  (*(volatile unsigned char *)0xDC03)

/** C64 matrix: A = row 1, col 2; D = row 2, col 2 (C64-Wiki). */
static bool key_held_a(void) {
    CIA1_PRA = (unsigned char)~0x02u;  /* row 1 */
    return (CIA1_PRB & 0x04u) == 0u;
}
static bool key_held_d(void) {
    CIA1_PRA = (unsigned char)~0x04u;  /* row 2 */
    return (CIA1_PRB & 0x04u) == 0u;
}

/**
 * @brief Read joystick 2 state from CIA1 port A.
 *
 * Assumptions (C64 common convention):
 * - We read **joystick port 2** via CIA1 port A ($DC00).
 * - Lines are **active low** (0 = pressed).
 * - Bit mapping:
 *   - bit0: Up
 *   - bit1: Down
 *   - bit2: Left
 *   - bit3: Right
 *   - bit4: Fire
 *
 * Notes:
 * - The keyboard matrix scan also writes to $DC00 (PRA) to select rows.
 *   For joystick reads we temporarily ensure DDRA is input and read $DC00.
 * - This is intentionally minimal for Phase 3; mapping is handled in input_poll().
 */
static unsigned char joystick2_state(void) {
    /*
     * Port A is shared: keyboard matrix uses it for row select (output), joystick
     * reads the same lines (input). Briefly set DDRA to input, read PRA, then
     * restore DDRA to output so the next keyboard scan works.
     */
    CIA1_DDRA = 0x00u;
    {
        unsigned char joy = CIA1_PRA;
        CIA1_DDRA = 0xFFu;
        return joy;
    }
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

        /* Joystick support (minimal mapping, joystick 2). */
        {
            unsigned char joy = joystick2_state();
            if ((joy & 0x04u) == 0u) frame.move_left_held = true;
            if ((joy & 0x08u) == 0u) frame.move_right_held = true;
            if ((joy & 0x02u) == 0u) frame.soft_drop_held = true;
            if ((joy & 0x10u) == 0u) frame.rotate_cw_held = true;
        }

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

/**
 * Joystick bits used for "any input" (title screen): Up, Down, Left, Right, Fire.
 */
#define JOY_ANY_MASK 0x1Fu

/** Previous joystick "any" state for title-screen edge detection. */
static bool g_prev_joy_any = true;

bool input_any_key_poll(void) {
    SCNKEY();
    if (GETIN() != 0) return true;
    /* Joystick: edge-triggered (one dismiss per press), aligned with key semantics. */
    unsigned char joy = joystick2_state();
    bool any = (joy & JOY_ANY_MASK) != JOY_ANY_MASK;
    bool edge = any && !g_prev_joy_any;
    g_prev_joy_any = any;
    return edge;
}

/** PETSCII RETURN. */
#define PETSCII_RETURN 13u
/** PETSCII '0'..'9'. */
#define PETSCII_0      0x30u
#define PETSCII_9      0x39u

/** Previous joystick state for start/help edge detection (left, right, fire). */
static bool g_prev_joy_left = false;
static bool g_prev_joy_right = false;
static bool g_prev_joy_fire = false;

void input_poll_start_help(uint8_t *digit, bool *start, bool *level_down, bool *level_up) {
    SCNKEY();
    {
        unsigned char c = GETIN();
        if (c != 0) {
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
            return;
        }
        /* No key: joystick as alias, edge-triggered (one action per press). */
        unsigned char joy = joystick2_state();
        bool left = (joy & 0x04u) == 0u;
        bool right = (joy & 0x08u) == 0u;
        bool fire = (joy & 0x10u) == 0u;
        if (left && !g_prev_joy_left && level_down != NULL) *level_down = true;
        if (right && !g_prev_joy_right && level_up != NULL) *level_up = true;
        if (fire && !g_prev_joy_fire) *start = true;
        g_prev_joy_left = left;
        g_prev_joy_right = right;
        g_prev_joy_fire = fire;
    }
}

/** Previous joystick fire for game-over edge detection. */
static bool g_prev_joy_fire_return = false;

bool input_return_pressed(void) {
    SCNKEY();
    if (GETIN() == PETSCII_RETURN) return true;
    /* Joystick fire as alias for RETURN, edge-triggered. */
    unsigned char joy = joystick2_state();
    bool fire = (joy & 0x10u) == 0u;
    bool edge = fire && !g_prev_joy_fire_return;
    g_prev_joy_fire_return = fire;
    return edge;
}
