/**
 * @file ascii_renderer.h
 * @brief Textual board rendering for host-side debugging.
 */

#ifndef QUATTRO_HOST_DEBUG_ASCII_RENDERER_H
#define QUATTRO_HOST_DEBUG_ASCII_RENDERER_H

#include "game_state.h"

/**
 * Renders the current board and active piece to stdout.
 * Empty cells: '.', locked cells: '1'..'7' by piece kind, active: 'A'.
 */
void ascii_render_board(const GameState *state);

/**
 * Prints next piece preview (single line) and score/level/lines.
 */
void ascii_render_status(const GameState *state);

#endif
