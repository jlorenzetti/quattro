/**
 * @file main.c
 * @brief C64 entry point and app-state loop for Quattro (Phase 2.6).
 *
 * Flow: Title → StartHelp → Game → GameOver (with replay prompt) → StartHelp → ...
 */

#include <stdint.h>

#include "game_state.h"
#include "gravity.h"
#include "input.h"
#include "timing.h"
#include "types.h"
#include "video.h"

/** Fixed seed for this slice (replay, debug, comparison with host). */
#define QUATTRO_C64_SEED 12345u

typedef enum {
    APP_TITLE,
    APP_START_HELP,
    APP_GAME,
    APP_GAME_OVER
} AppState;

int main(void) {
    GameState state;
    uint16_t gravity_counter = 0;
    AppState app_state = APP_TITLE;
    uint8_t start_level = 5;
    uint8_t start_help_last_drawn = 255;
    unsigned int start_help_pending_draw = 1;
    unsigned int game_over_drawn = 0;

    video_init();

    for (;;) {
        switch (app_state) {
        case APP_TITLE:
            video_draw_title();
            while (!input_any_key_poll()) {
                timing_wait_frame();
            }
            start_help_pending_draw = 1;
            app_state = APP_START_HELP;
            break;

        case APP_START_HELP:
            if (start_help_pending_draw || start_help_last_drawn != start_level) {
                video_draw_start_help(start_level);
                start_help_last_drawn = start_level;
                start_help_pending_draw = 0;
            }
            timing_wait_frame();
            {
                uint8_t digit = 255;
                bool start = false;
                input_poll_start_help(&digit, &start);
                if (start) {
                    video_clear();
                    game_start(&state, QUATTRO_C64_SEED, start_level);
                    gravity_counter = 0;
                    app_state = APP_GAME;
                } else if (digit <= 9) {
                    start_level = digit;
                }
            }
            break;

        case APP_GAME:
            if (!game_is_over(&state)) {
                Command cmd = input_poll();
                game_apply_command(&state, cmd);
                video_draw_frame();
                video_draw_board(&state);
                video_draw_hud(&state);
                timing_wait_frame();
                {
                    uint16_t interval = gravity_interval_for_level(state.score.level);
                    gravity_counter++;
                    if (gravity_counter >= interval) {
                        gravity_counter = 0;
                        game_tick_gravity(&state);
                    }
                }
            } else {
                game_over_drawn = 0;
                app_state = APP_GAME_OVER;
            }
            break;

        case APP_GAME_OVER:
            if (!game_over_drawn) {
                video_draw_frame();
                video_draw_board(&state);
                video_draw_hud(&state);
                video_draw_game_over();
                video_draw_replay_prompt();
                game_over_drawn = 1;
            }
            timing_wait_frame();
            if (input_return_pressed()) {
                start_help_pending_draw = 1;
                app_state = APP_START_HELP;
            }
            break;
        }
    }
    return 0;
}
