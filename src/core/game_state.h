#ifndef QUATTRO_CORE_GAME_STATE_H
#define QUATTRO_CORE_GAME_STATE_H

#include "types.h"

/** Starts a new game with given seed and start level (0–9). */
void game_start(GameState *state, uint32_t seed, uint8_t start_level);
/** @param step_result optional; if non-NULL, written with rotate/lock/clear telemetry for this call. */
void game_apply_command(GameState *state, Command command, GameStepResult *step_result);
/** @param step_result optional; if non-NULL, zeroed at entry; remains all zero if the piece only descends; on lock, `locked` and `lines_cleared` are set. */
void game_tick_gravity(GameState *state, GameStepResult *step_result);
bool game_is_over(const GameState *state);

#endif
