/**
 * @file game_state.c
 * @brief Game state machine: start, commands, gravity tick, game over.
 */

#include "board.h"
#include "game_state.h"
#include "rng.h"
#include "rules.h"
#include "scoring.h"

static void spawn_next(GameState *state) {
    state->active = rules_make_spawn_piece(state->next_piece);
    state->next_piece = rng_next_piece(&state->rng);
}

void game_start(GameState *state, uint32_t seed) {
    if (!state) return;
    board_clear(&state->board);
    scoring_reset(&state->score);
    state->phase = GAME_PHASE_RUNNING;
    rng_seed(&state->rng, seed);
    state->next_piece = rng_next_piece(&state->rng);
    spawn_next(state);
    if (!rules_can_place_piece(&state->board, state->active.kind, state->active.rotation,
                              state->active.x, state->active.y)) {
        state->phase = GAME_PHASE_GAME_OVER;
    }
}

void game_apply_command(GameState *state, Command command) {
    if (!state || state->phase == GAME_PHASE_GAME_OVER) return;
    switch (command) {
        case CMD_NONE:
            break;
        case CMD_MOVE_LEFT:
            if (rules_can_move(&state->board, &state->active, -1, 0)) state->active.x--;
            break;
        case CMD_MOVE_RIGHT:
            if (rules_can_move(&state->board, &state->active, 1, 0)) state->active.x++;
            break;
        case CMD_ROTATE_CW: {
            Rotation target = rotation_rotate_cw(state->active.rotation);
            if (rules_can_rotate(&state->board, &state->active, target)) state->active.rotation = target;
            break;
        }
        case CMD_ROTATE_CCW: {
            Rotation target = rotation_rotate_ccw(state->active.rotation);
            if (rules_can_rotate(&state->board, &state->active, target)) state->active.rotation = target;
            break;
        }
        case CMD_SOFT_DROP:
            if (rules_can_move(&state->board, &state->active, 0, 1)) state->active.y++;
            break;
    }
}

void game_tick_gravity(GameState *state) {
    if (!state || state->phase == GAME_PHASE_GAME_OVER) return;
    if (rules_can_move(&state->board, &state->active, 0, 1)) {
        state->active.y++;
        return;
    }
    rules_lock_active_piece(&state->board, &state->active);
    uint8_t cleared = board_clear_full_rows(&state->board);
    if (cleared > 0) {
        scoring_apply_line_clear(&state->score, cleared);
    }
    spawn_next(state);
    if (!rules_can_place_piece(&state->board, state->active.kind, state->active.rotation,
                              state->active.x, state->active.y)) {
        state->phase = GAME_PHASE_GAME_OVER;
    }
}

bool game_is_over(const GameState *state) {
    return state && state->phase == GAME_PHASE_GAME_OVER;
}
