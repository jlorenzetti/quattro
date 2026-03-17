/**
 * @file test_input_model.c
 * @brief Tests for Quattro-owned input model (Phase 3).
 */

#include <stddef.h>
#include <stdbool.h>

#include "input_model.h"

static int expect(Command got, Command expected) {
    return got == expected ? 0 : 1;
}

int test_input_model_run(void) {
    InputModel m;
    input_model_init(&m);

    /* Rotation remains edge-triggered. */
    {
        InputFrame f = {0};
        f.rotate_cw_held = true;
        if (expect(input_model_step(&m, &f), CMD_ROTATE_CW)) return 11;
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 12;
        f.rotate_cw_held = false;
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 13;
    }

    input_model_reset(&m);

    /*
     * Left/right conflict:
     * - newest held direction wins
     * - no oscillation while both held
     * - handoff on release with fresh DAS (treated as new press: immediate step, then delay)
     */
    {
        InputFrame f = {0};

        /* Press and hold LEFT: immediate step. */
        f.move_left_held = true;
        if (expect(input_model_step(&m, &f), CMD_MOVE_LEFT)) return 21;

        /* Tap RIGHT while still holding LEFT: newest wins, immediate RIGHT step. */
        f.move_right_held = true;
        if (expect(input_model_step(&m, &f), CMD_MOVE_RIGHT)) return 22;

        /* While both held, must not emit LEFT (no jitter). */
        for (int i = 0; i < 20; i++) {
            Command c = input_model_step(&m, &f);
            if (c == CMD_MOVE_LEFT) return 23;
        }

        /* Release RIGHT while LEFT remains held: handoff back to LEFT. */
        f.move_right_held = false;
        if (expect(input_model_step(&m, &f), CMD_MOVE_LEFT)) return 24;

        /* Fresh DAS: no further lateral moves for 11 frames after the handoff press frame. */
        for (int i = 0; i < 11; i++) {
            Command c = input_model_step(&m, &f);
            if (c == CMD_MOVE_LEFT || c == CMD_MOVE_RIGHT) return 25;
        }
    }

    input_model_reset(&m);

    /* Lateral tuning: repeat interval is 3 frames, delay remains 12. */
    {
        InputFrame f = {0};
        f.move_left_held = true;

        if (expect(input_model_step(&m, &f), CMD_MOVE_LEFT)) return 31;

        /* No auto-repeat during the next 11 frames (held frames 2..12). */
        for (int i = 0; i < 11; i++) {
            if (expect(input_model_step(&m, &f), CMD_NONE)) return 32;
        }

        /* First repeat step occurs when held frames become 13. */
        if (expect(input_model_step(&m, &f), CMD_MOVE_LEFT)) return 33;

        /* Then repeats every 3 frames. */
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 34;
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 35;
        if (expect(input_model_step(&m, &f), CMD_MOVE_LEFT)) return 36;
    }

    input_model_reset(&m);

    /* Soft drop tuning: delay 3, repeat interval 1. */
    {
        InputFrame f = {0};
        f.soft_drop_held = true;

        /* First press is immediate 1-cell soft drop. */
        if (expect(input_model_step(&m, &f), CMD_SOFT_DROP)) return 41;

        /* Delay: next 2 frames should be none (held frames 2..3). */
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 42;
        if (expect(input_model_step(&m, &f), CMD_NONE)) return 43;

        /* Held frame 4 starts repeat; interval 1 means every frame thereafter. */
        if (expect(input_model_step(&m, &f), CMD_SOFT_DROP)) return 44;
        if (expect(input_model_step(&m, &f), CMD_SOFT_DROP)) return 45;
        if (expect(input_model_step(&m, &f), CMD_SOFT_DROP)) return 46;
    }

    return 0;
}

