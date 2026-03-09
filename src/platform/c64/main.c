/**
 * @file main.c
 * @brief C64 entry point and minimal game loop for Quattro.
 */

#include "game_state.h"
#include "input.h"
#include "timing.h"
#include "types.h"
#include "video.h"

/** Fixed seed for this slice (replay, debug, comparison with host). */
#define QUATTRO_C64_SEED 12345u

/** Gravity every N frames (observable, not tuned). */
#define GRAVITY_EVERY_N_FRAMES 20u

int main(void) {
    GameState state;
    uint16_t frame = 0;

    video_init();
    game_start(&state, QUATTRO_C64_SEED);

    while (!game_is_over(&state)) {
        Command cmd = input_poll();
        game_apply_command(&state, cmd);
        video_draw_frame();
        video_draw_board(&state);
        video_draw_hud(&state);
        timing_wait_frame();
        frame++;
        if (frame % GRAVITY_EVERY_N_FRAMES == 0) {
            game_tick_gravity(&state);
        }
    }

    video_draw_game_over();
    for (;;) { /* game over: halt */ }
    return 0;
}
