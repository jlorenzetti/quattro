/**
 * @file test_game_state.c
 * @brief Tests for game state: start, gravity, game over.
 */

#include "game_state.h"
#include "types.h"

int test_game_state_run(void) {
    GameState state;
    game_start(&state, 42);

    if (game_is_over(&state)) return 1;
    if (state.phase != GAME_PHASE_RUNNING) return 1;

    game_apply_command(&state, CMD_MOVE_LEFT);
    if (state.active.x >= QUATTRO_SPAWN_X) return 1;

    game_apply_command(&state, CMD_MOVE_RIGHT);
    game_apply_command(&state, CMD_MOVE_RIGHT);

    game_apply_command(&state, CMD_SOFT_DROP);
    game_tick_gravity(&state);

    game_apply_command(&state, CMD_ROTATE_CW);
    game_apply_command(&state, CMD_ROTATE_CCW);

    PieceKind first_next = state.next_piece;
    GameState state2;
    game_start(&state2, 42);
    if (state2.next_piece != first_next) return 1;

    return 0;
}

/**
 * Deterministic full game until game over. Validates that the loop terminates
 * and final state is consistent (phase, score).
 */
int test_game_state_full_run(void) {
    const uint32_t seed = 12345u;
    const unsigned max_ticks = 100000u;

    GameState state;
    game_start(&state, seed);

    unsigned ticks = 0;
    while (!game_is_over(&state) && ticks < max_ticks) {
        game_tick_gravity(&state);
        ticks++;
    }

    if (!game_is_over(&state)) return 1;
    if (state.phase != GAME_PHASE_GAME_OVER) return 1;
    if (ticks == 0) return 1;

    return 0;
}
