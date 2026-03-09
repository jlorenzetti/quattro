#ifndef QUATTRO_CORE_GAME_STATE_H
#define QUATTRO_CORE_GAME_STATE_H

#include "types.h"

void game_start(GameState *state, uint32_t seed);
void game_apply_command(GameState *state, Command command);
void game_tick_gravity(GameState *state);
bool game_is_over(const GameState *state);

#endif
