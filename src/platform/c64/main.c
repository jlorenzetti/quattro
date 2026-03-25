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
#include "seed.h"
#include "timing.h"
#include "types.h"
#include "video.h"

/* Optional CIA2 section timing + screen dump; only `make c64_perf` defines QUATTRO_PERF. */
#ifdef QUATTRO_PERF
#include "perf.h"
#endif

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
    uint16_t start_help_frames = 0;
    unsigned int game_frame_drawn = 0;

    video_init();
    input_init();
#ifdef QUATTRO_PERF
    perf_init();
#endif

    for (;;) {
        switch (app_state) {
        case APP_TITLE:
            video_draw_title();
            while (!input_any_key_poll()) {
                timing_wait_frame();
            }
            start_help_pending_draw = 1;
            start_help_frames = 0;
            input_reset_start_help();
            app_state = APP_START_HELP;
            break;

        case APP_START_HELP:
            start_help_frames++;
            if (start_help_pending_draw || start_help_last_drawn != start_level) {
                video_draw_start_help(start_level);
                start_help_last_drawn = start_level;
                start_help_pending_draw = 0;
            }
            timing_wait_frame();
            {
                uint8_t digit = 255;
                bool start = false;
                bool level_down = false;
                bool level_up = false;
                input_poll_start_help(&digit, &start, &level_down, &level_up);
                if (start) {
                    video_clear();
                    game_start(&state, seed_generate(start_level, start_help_frames), start_level);
                    input_reset_gameplay();
                    gravity_counter = 0;
                    game_frame_drawn = 0;
                    app_state = APP_GAME;
                } else if (digit <= 9) {
                    start_level = digit;
                } else if (level_down && start_level > 0) {
                    start_level--;
                } else if (level_up && start_level < 9) {
                    start_level++;
                }
            }
            break;

        case APP_GAME:
            if (!game_is_over(&state)) {
#ifdef QUATTRO_PERF
                perf_section_start(PERF_INPUT);
#endif
                Command cmd = input_poll();
#ifdef QUATTRO_PERF
                perf_section_end(PERF_INPUT);
                perf_section_start(PERF_APPLY);
#endif
                game_apply_command(&state, cmd);
#ifdef QUATTRO_PERF
                perf_section_end_apply(cmd);
#endif
                if (!game_frame_drawn) {
                    video_draw_frame();
                    game_frame_drawn = 1;
                }
#ifdef QUATTRO_PERF
                perf_section_start(PERF_BOARD);
#endif
                video_draw_board(&state);
#ifdef QUATTRO_PERF
                perf_section_end(PERF_BOARD);
                perf_section_start(PERF_HUD);
#endif
                video_draw_hud(&state);
#ifdef QUATTRO_PERF
                perf_section_end(PERF_HUD);
#endif
                timing_wait_frame();
                {
                    uint16_t interval = gravity_interval_for_level(state.score.level);
                    gravity_counter++;
                    if (gravity_counter >= interval) {
                        gravity_counter = 0;
#ifdef QUATTRO_PERF
                        perf_section_start(PERF_GRAVITY);
#endif
                        game_tick_gravity(&state);
#ifdef QUATTRO_PERF
                        perf_section_end(PERF_GRAVITY);
#endif
                    }
                }
#ifdef QUATTRO_PERF
                perf_frame_end();
                if (perf_halted()) {
                    while (1) {
                        timing_wait_frame();
                    }
                }
#endif
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
                start_help_frames = 0;
                input_reset_start_help();
                app_state = APP_START_HELP;
            }
            break;
        }
    }
    return 0;
}
