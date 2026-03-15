/**
 * @file ascii_renderer.h
 * @brief Textual board rendering for host-side debugging.
 */

#ifndef QUATTRO_HOST_DEBUG_ASCII_RENDERER_H
#define QUATTRO_HOST_DEBUG_ASCII_RENDERER_H

#include "game_state.h"

/**
 * Prints LINES, LEVEL, SCORE (fixed-width, top of screen).
 */
void ascii_render_status(const GameState *state);

/**
 * Renders the board with <! !> borders. Empty cells: " .", filled: "[]".
 */
void ascii_render_board(const GameState *state);

#endif
